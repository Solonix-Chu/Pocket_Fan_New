/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#include "../hal_pocketfan.h"
#include "hal_display.h"
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <esp_timer.h>

static const char *TAG = "hal_power";

#define PIN_POWER_HOLD GPIO_NUM_45  // 高电平保持供电，低电平断电
#define PIN_POWER_BTN  GPIO_NUM_46

// void HAL_PocketFan::powerOn()
static void powerOn()
{
    ESP_LOGI(TAG, "Powering on...");
    // Release power hold
    gpio_set_level(PIN_POWER_HOLD, 1);
}

void HAL_PocketFan::powerOff()
{
    ESP_LOGI(TAG, "Powering off...");

    esp_lcd_panel_handle_t panel_handle = hal_display_get_panel_handle();
    if (panel_handle) {
        (void)esp_lcd_panel_disp_on_off(panel_handle, false);
    }

    // Give the command a moment to reach the panel before we cut power.
    vTaskDelay(pdMS_TO_TICKS(10));

    // Release power hold
    gpio_set_level(PIN_POWER_HOLD, 0);
    
    // Wait for death
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    ESP_LOGE(TAG, "Power off failed (external power?), restarting...");
    esp_restart();
}

void HAL_PocketFan::_power_init()
{
    ESP_LOGI(TAG, "Power init");

    // Config Power Hold Pin
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_POWER_HOLD);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    gpio_set_level(PIN_POWER_HOLD, 0);

    // Config Power Button Pin
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << PIN_POWER_BTN);
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    gpio_config(&io_conf);

    // Power-on gate:
    // Require a continuous long-press (>2s) on the OK/Power key (GPIO46) to boot.
    // During this period the display is not initialized yet, so it stays black as requested.
    static constexpr int64_t k_boot_press_us = 100; // 2s 由于启动耗时（bootloader + 初始化）已经接近2s，这里改为0.1s以免用户等待过久，体感接近2s
    auto is_pressed = []() -> bool {
        // Active-low
        return gpio_get_level(PIN_POWER_BTN) == 0;
    };

    ESP_LOGI(TAG, "Hold OK/Power key >= 2s to boot (GPIO%d).", (int)PIN_POWER_BTN);
    while (true) {
        // Wait for press
        while (!is_pressed()) {
            vTaskDelay(pdMS_TO_TICKS(20));
        }

        // Measure continuous hold
        const int64_t start_us = esp_timer_get_time();
        while (is_pressed()) {
            if ((esp_timer_get_time() - start_us) >= k_boot_press_us) {
                goto boot_confirmed;
            }
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // Released early: ignore and continue waiting (on battery the board will likely cut power).
        ESP_LOGI(TAG, "Power key released before 2s; waiting again...");
    }

boot_confirmed:
    ESP_LOGI(TAG, "Power on confirmed. Holding power.");
    powerOn();
    // Note: Do not block waiting for release here. After the 2s gate, we proceed
    // with boot and show the startup screen even if the user is still holding.
    // Power-off long-press is armed only after the first release in app layer.
}
