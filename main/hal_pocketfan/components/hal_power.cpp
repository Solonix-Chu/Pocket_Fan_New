/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#include "../hal_pocketfan.h"
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

    if (gpio_get_level(PIN_POWER_BTN) == 1) {
        ESP_LOGI(TAG, "Power button not pressed at startup. Checking if we should stay on (e.g. charging?) - For now enforcing button rule.");
    }

    ESP_LOGI(TAG, "Checking power on long press...");
    int64_t start_time = esp_timer_get_time();
    while ((esp_timer_get_time() - start_time) < 3000000) {  // 3s
        if (gpio_get_level(PIN_POWER_BTN) == 1) {
             ESP_LOGI(TAG, "Power button released early (%lld ms).", (esp_timer_get_time() - start_time) / 1000);
             powerOff();
             // If powerOff returns (e.g. on USB), we should probably stop init.
             while(1) vTaskDelay(pdMS_TO_TICKS(100)); 
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ESP_LOGI(TAG, "Power on confirmed. Holding power.");
    powerOn();
    
    // Release the button pin so app_button (iot_button) can take over
    gpio_reset_pin(PIN_POWER_BTN);
}
