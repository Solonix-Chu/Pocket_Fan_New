/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <driver/ledc.h>
#include <esp_log.h>

static const char *TAG = "HAL_Motor";

#define MOTOR_PIN 5
#define MOTOR_PWM_FREQ 20000
#define MOTOR_PWM_RES LEDC_TIMER_10_BIT
#define MOTOR_LEDC_CHANNEL LEDC_CHANNEL_0

static bool _motor_enabled = false;
static float _current_speed = 0.0f;

#include <driver/gpio.h>

void HAL_PocketFan::_motor_init()
{
    ESP_LOGI(TAG, "Motor Init on Pin %d", MOTOR_PIN);
    
    // Reset pin to default state
    gpio_reset_pin((gpio_num_t)MOTOR_PIN);

    // Timer Config
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = MOTOR_PWM_RES,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = MOTOR_PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure      = false,
    };
    esp_err_t ret = ledc_timer_config(&ledc_timer);
    if (ret != ESP_OK) ESP_LOGE(TAG, "LEDC Timer Config Failed: %s", esp_err_to_name(ret));

    // Channel Config
    ledc_channel_config_t ledc_channel = {
        .gpio_num       = MOTOR_PIN,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = MOTOR_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0,
        .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags          = { .output_invert = 0 },
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) ESP_LOGE(TAG, "LEDC Channel Config Failed: %s", esp_err_to_name(ret));
}

void HAL_PocketFan::setFanState(bool enable)
{
    _motor_enabled = enable;
    ESP_LOGI(TAG, "Fan State: %s", enable ? "ON" : "OFF");
    if (enable) {
        setFanSpeed(_current_speed);
    } else {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL);
    }
}

void HAL_PocketFan::setFanSpeed(float speed)
{
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    _current_speed = speed;

    if (_motor_enabled) {
        uint32_t duty = (uint32_t)(speed * 1023.0f); // 10-bit resolution
        ESP_LOGI(TAG, "Set Fan Speed: %.2f (Duty: %lu)", speed, duty);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL, duty);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL);
    }
}
