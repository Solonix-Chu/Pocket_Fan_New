/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "errno.h"
#include "usr_lcd.h"
// #include "usr_nimble.h"
// #include "mcu_info.h"
#include "app_button.h"
#include "app_ui.h"

static const char *TAG = "app_main";

void app_main(void)
{
    ESP_LOGI(TAG, "Initial free heap size: %d", (int)esp_get_free_heap_size());

    // Initialize components
    app_button_init();
    usr_lcd_init();
    
    // Initialize UI logic
    ui_init();

    ESP_LOGI(TAG, "Heap size after init: %d", (int)esp_get_free_heap_size());

    // Create UI task
    xTaskCreate(ui_task, "ui_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "UI task created.");

    // The app_main can exit now, or do other things.
    // The UI will run in its own task.
}
