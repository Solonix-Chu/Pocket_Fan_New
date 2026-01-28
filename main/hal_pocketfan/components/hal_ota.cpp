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
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <ble_ota.h>

// Global semaphore required by espressif__ble_ota component (nimble_ota.c)
SemaphoreHandle_t notify_sem = NULL;

static const char *TAG = "HAL_OTA";
static esp_ota_handle_t _ota_handle = 0;
static const esp_partition_t *_update_partition = NULL;
static bool _is_ota_started = false;
static uint32_t _received_len = 0;

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
    }

    // Write data
    err = esp_ota_write(_ota_handle, buf, length);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_ota_write failed (%s)", esp_err_to_name(err));
        return;
    }
    
    _received_len += length;
    uint32_t total_len = esp_ble_ota_get_fw_length();
    
    ESP_LOGI(TAG, "OTA Progress: %lu / %lu", _received_len, total_len);

    // Check if finished
    if (_received_len >= total_len && total_len > 0) {
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
    ESP_LOGI(TAG, "Starting BLE OTA Service...");
    
        // Create the semaphore required by the component
        if (notify_sem == NULL) {
            notify_sem = xSemaphoreCreateBinary();
            xSemaphoreGive(notify_sem);
        }
    
        // Release Classic BT memory (save RAM)
        ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
        // Initialize NVS.
        esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK(nvs_flash_erase());
            ret = nvs_flash_init();
        }
        ESP_ERROR_CHECK(ret);
    
        // Initialize BLE Controller (Required for NimBLE)
        esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
        ret = esp_bt_controller_init(&bt_cfg);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_bt_controller_init failed (%s)", esp_err_to_name(ret));
            return;
        }
    
        ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_bt_controller_enable failed (%s)", esp_err_to_name(ret));
            return;
        }
    
        // Initialize BLE OTA Host (this initializes NimBLE stack)
        ret = esp_ble_ota_host_init();
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "esp_ble_ota_host_init failed (%s)", esp_err_to_name(ret));
            return;
        }
    
        // Register callback
        esp_ble_ota_recv_fw_data_callback(_ota_recv_fw_cb);
    
        ESP_LOGI(TAG, "BLE OTA Ready. Waiting for connection...");
    
        // Get MAC
        uint8_t mac[6];
        esp_read_mac(mac, ESP_MAC_BT);
        char mac_str[18];
        snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

        lv_obj_t *ota_screen = nullptr;
        lv_obj_t *label_title = nullptr;
        lv_obj_t *label_name = nullptr;
        lv_obj_t *label_mac = nullptr;
        lv_obj_t *label_status = nullptr;
        lv_obj_t *label_progress = nullptr;

        if (lv_is_initialized()) {
            ota_screen = lv_obj_create(NULL);
            lv_obj_clear_flag(ota_screen, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(ota_screen, lv_color_black(), 0);
            lv_obj_set_style_bg_opa(ota_screen, LV_OPA_COVER, 0);

            label_title = lv_label_create(ota_screen);
            lv_label_set_text(label_title, "BLE OTA Ready");
            lv_obj_set_pos(label_title, 4, 2);

            label_name = lv_label_create(ota_screen);
            lv_label_set_text(label_name, "Name: nimble-ble-ota");
            lv_obj_set_pos(label_name, 4, 14);

            label_mac = lv_label_create(ota_screen);
            lv_label_set_text(label_mac, mac_str);
            lv_obj_set_pos(label_mac, 4, 26);

            label_status = lv_label_create(ota_screen);
            lv_obj_set_pos(label_status, 4, 38);

            label_progress = lv_label_create(ota_screen);
            lv_obj_set_pos(label_progress, 4, 50);

            lv_obj_set_style_text_color(label_title, lv_color_white(), 0);
            lv_obj_set_style_text_color(label_name, lv_color_white(), 0);
            lv_obj_set_style_text_color(label_mac, lv_color_white(), 0);
            lv_obj_set_style_text_color(label_status, lv_color_white(), 0);
            lv_obj_set_style_text_color(label_progress, lv_color_white(), 0);
            lv_scr_load(ota_screen);
        }

        // Enter Blocking Loop for OTA
        while (true) {
            if (label_status) {
                if (_is_ota_started) {
                    lv_label_set_text(label_status, "Writing Firmware...");
                    uint32_t total_len = esp_ble_ota_get_fw_length();
                    if (total_len > 0) {
                        lv_label_set_text_fmt(label_progress, "%lu/%lu (%lu%%)", _received_len, total_len,
                                              (_received_len * 100) / total_len);
                    } else {
                        lv_label_set_text_fmt(label_progress, "%lu bytes", _received_len);
                    }
                } else {
                    lv_label_set_text(label_status, "Waiting connect...");
                    lv_label_set_text(label_progress, "");
                }
            }

            HAL::LGVL_UPDATE();

            // Feed watchdog
            FeedTheDog();

            // Delay
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
