/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#include "../hal_pocketfan.h"
#include "esp_system.h"
#include <mooncake.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <esp_task_wdt.h>
#include "esp_log.h"
static const char *TAG = "hal_watch_dog";
// Refs:
// https://github.com/espressif/esp-idf/blob/v5.2.1/examples/system/task_watchdog/main/task_watchdog_example_main.c
// https://docs.espressif.com/projects/esp-idf/en/v5.2.1/esp32/api-reference/system/wdts.html

void HAL_PocketFan::_watch_dog_init()
{
    ESP_LOGI(TAG, "Watch dog init");

    // Deinitialize TWDT first to apply new configuration
    esp_task_wdt_deinit();
    
    // Init twdt
    esp_task_wdt_config_t twdt_config = {
        .timeout_ms = 5000, // Increased timeout for debugging
        // .idle_core_mask = (1 << portNUM_PROCESSORS) - 1, // Bitmask of all cores
        .idle_core_mask = 1 << 1, // Only IDLE1
        .trigger_panic = true,
    };
    esp_err_t ret = esp_task_wdt_init(&twdt_config);
    if (ret != ESP_OK) {
        // If it's still INVALID_STATE after deinit, something is fundamentally wrong, or another component re-inited it.
        ESP_ERROR_CHECK(ret); 
    }

    // Subscribes main task
    // ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    ret = esp_task_wdt_add(NULL);
    if (ret != ESP_OK) {
        if (ret == ESP_ERR_INVALID_STATE) {
             ESP_LOGW(TAG, "Task already subscribed to TWDT");
        } else {
             // ESP_ERROR_CHECK(ret); // Don't crash if we can't add, just warn
             ESP_LOGE(TAG, "Failed to subscribe task to TWDT: %s", esp_err_to_name(ret));
        }
    }
    // ESP_ERROR_CHECK(esp_task_wdt_status(NULL));

    /* -------------------------------------------------------------------------- */
    /*                                    Test                                    */
    /* -------------------------------------------------------------------------- */
    // int b = 2;
    // while (1)
    // {
    //     // delay(5);
    //     int a = b * b;
    //     feedTheDog();
    // }
}

void HAL_PocketFan::feedTheDog() { esp_task_wdt_reset(); }

void HAL_PocketFan::reboot() { esp_restart(); }
