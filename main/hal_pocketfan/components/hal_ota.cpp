/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <esp_bt.h>
#include <esp_mac.h>
#include <esp_event.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <ble_ota.h>
#include <nimble/nimble_port.h>

// Global semaphore required by espressif__ble_ota component (nimble_ota.c)
SemaphoreHandle_t notify_sem = NULL;

static const char *TAG = "HAL_OTA";
static esp_ota_handle_t _ota_handle = 0;
static const esp_partition_t *_update_partition = NULL;
static bool _is_ota_started = false;
static uint32_t _received_len = 0;
static uint32_t _total_len = 0;
static bool _ble_ota_ready = false;
static esp_err_t _ble_ota_init_error = ESP_OK;
static bool _nimble_started = false;

static void _ble_ota_controller_cleanup()
{
    const auto status = esp_bt_controller_get_status();
    if (status == ESP_BT_CONTROLLER_STATUS_ENABLED) {
        esp_err_t err = esp_bt_controller_disable();
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "esp_bt_controller_disable failed (%s)", esp_err_to_name(err));
        }
    }
    if (status == ESP_BT_CONTROLLER_STATUS_ENABLED || status == ESP_BT_CONTROLLER_STATUS_INITED) {
        esp_err_t err = esp_bt_controller_deinit();
        if (err != ESP_OK) {
            ESP_LOGW(TAG, "esp_bt_controller_deinit failed (%s)", esp_err_to_name(err));
        }
    }
}

static void _ota_recv_fw_cb(uint8_t *buf, uint32_t length)
{
    esp_err_t err;
    
    // First packet: Start OTA
    if (!_is_ota_started) {
        ESP_LOGI(TAG, "Starting OTA Write...");
        _update_partition = esp_ota_get_next_update_partition(NULL);
        if (_update_partition == NULL) {
            ESP_LOGE(TAG, "No update partition found");
            return;
        }

        err = esp_ota_begin(_update_partition, OTA_SIZE_UNKNOWN, &_ota_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
            return;
        }
        _is_ota_started = true;
        _received_len = 0;
        _total_len = esp_ble_ota_get_fw_length();
    }

    // Write data
    err = esp_ota_write(_ota_handle, buf, length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        return;
    }
    
    _received_len += length;
    _total_len = esp_ble_ota_get_fw_length();
    
    ESP_LOGI(TAG, "OTA Progress: %lu / %lu", _received_len, _total_len);

    // Check if finished
    if (_received_len >= _total_len && _total_len > 0) {
        ESP_LOGI(TAG, "OTA Complete. Validating...");
        
        err = esp_ota_end(_ota_handle);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_end failed (%s)", esp_err_to_name(err));
            return;
        }

        err = esp_ota_set_boot_partition(_update_partition);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)", esp_err_to_name(err));
            return;
        }

        ESP_LOGI(TAG, "OTA Success. Rebooting...");
        
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp_restart();
    }
}

void HAL_PocketFan::startBleOta()
{
    if (_ble_ota_ready) {
        return;
    }

    ESP_LOGI(TAG, "Starting BLE OTA Service...");

    esp_err_t ret = ESP_OK;

    // Reset state
    _is_ota_started = false;
    _received_len = 0;
    _total_len = 0;
    _ble_ota_init_error = ESP_OK;
    _nimble_started = false;

    // Create the semaphore required by the component
    if (notify_sem == NULL) {
        notify_sem = xSemaphoreCreateBinary();
        xSemaphoreGive(notify_sem);
    }

    // Ensure NVS is initialized (required by some BLE/NimBLE features depending on sdkconfig)
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_init failed (%s)", esp_err_to_name(ret));
        _ble_ota_init_error = ret;
        _ble_ota_ready = false;
        return;
    }

    // Release Classic BT memory (save RAM)
    if (esp_bt_controller_get_status() == ESP_BT_CONTROLLER_STATUS_IDLE) {
        ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
        if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
            ESP_LOGW(TAG, "esp_bt_controller_mem_release failed (%s)", esp_err_to_name(ret));
        }
    }

    // Ensure default event loop exists (required by some IDF components)
    ret = esp_event_loop_create_default();
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGW(TAG, "esp_event_loop_create_default failed (%s)", esp_err_to_name(ret));
    }

    // Initialize + enable BLE controller for VHCI (required before esp_nimble_hci_init -> esp_vhci_host_register_callback)
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_bt_controller_init failed (%s)", esp_err_to_name(ret));
        _ble_ota_init_error = ret;
        _ble_ota_ready = false;
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "esp_bt_controller_enable failed (%s)", esp_err_to_name(ret));
        _ble_ota_init_error = ret;
        _ble_ota_ready = false;
        _ble_ota_controller_cleanup();
        return;
    }

    // Initialize BLE OTA Host (starts NimBLE stack + advertising)
    ret = esp_ble_ota_host_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ble_ota_host_init failed (%s)", esp_err_to_name(ret));
        _ble_ota_init_error = ret;
        _ble_ota_ready = false;
        _ble_ota_controller_cleanup();
        return;
    }

    // Register callback
    ret = esp_ble_ota_recv_fw_data_callback(_ota_recv_fw_cb);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_ble_ota_recv_fw_data_callback failed (%s)", esp_err_to_name(ret));
        _ble_ota_init_error = ret;
        _ble_ota_ready = false;
        _ble_ota_controller_cleanup();
        return;
    }

    _ble_ota_ready = true;
    _nimble_started = true;
    ESP_LOGI(TAG, "BLE OTA Ready. Waiting for connection...");
}

bool HAL_PocketFan::stopBleOta()
{
    if (_is_ota_started) {
        ESP_LOGW(TAG, "stopBleOta ignored: OTA write in progress");
        return false;
    }

    if (_nimble_started) {
        esp_err_t ret = nimble_port_stop();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "nimble_port_stop failed (%s)", esp_err_to_name(ret));
        }

        ret = esp_nimble_deinit();
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "esp_nimble_deinit failed (%s)", esp_err_to_name(ret));
        }
    }

    _ble_ota_controller_cleanup();

    _ble_ota_ready = false;
    _nimble_started = false;
    _ble_ota_init_error = ESP_OK;
    return true;
}

BLE_OTA::Status HAL_PocketFan::getBleOtaStatus()
{
    BLE_OTA::Status status;
    if (!_ble_ota_ready) {
        status.state = BLE_OTA::STATE_IDLE;
        status.init_error = _ble_ota_init_error;
        return status;
    }

    status.state = _is_ota_started ? BLE_OTA::STATE_WRITING : BLE_OTA::STATE_READY;
    status.received_len = _received_len;
    status.total_len = _total_len;
    status.init_error = 0;
    return status;
}
