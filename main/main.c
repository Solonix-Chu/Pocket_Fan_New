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
static const char *TAG = "app_main";

extern int ets_printf(const char *fmt, ...);

void app_main(void)
{
    printf("free_heap_size = %d\n", (int)esp_get_free_heap_size());

    // usr_nimble_init();
    app_button_init();

    ESP_LOGI(TAG, "LCD test_display");
    usr_lcd_init();

    printf("free_heap_size = %d\n", (int)esp_get_free_heap_size());

    // Perform all example operations in a loop to allow USB reconnections
    while (1) {

    // 打印所有任务的状态信息
    // vTaskList(buffer);
    // printf("Task List:\n%s\n", buffer);

    // free(buffer);  // 释放缓冲区

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
