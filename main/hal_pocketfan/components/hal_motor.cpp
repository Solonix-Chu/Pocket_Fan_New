/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <driver/ledc.h>
#include <esp_log.h>
#include <esp_err.h>
#include <driver/gpio.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

static const char *TAG = "HAL_Motor";

#define MOTOR_PIN 5
#define MOTOR_MOS_EN_PIN GPIO_NUM_10
// The schematic and demo label GPIO5 as "PWM OUT" and GPIO10 as "ESC MOS switch".
// For most ESCs this is RC-servo style PWM (50~400Hz, 1~2ms pulse width), not
// high-frequency duty PWM for a DC motor.
#define ESC_PWM_FREQ_HZ 50
#define ESC_PWM_RES     LEDC_TIMER_14_BIT
#define ESC_PULSE_MIN_US 1000
#define ESC_PULSE_MAX_US 2000
// Give the ESC time to power up and arm at minimum throttle before applying target.
#define ESC_POWERUP_DELAY_MS 300
#define ESC_ARM_DELAY_MS     700
// Avoid conflicts with RGBW LED driver (uses TIMER_0 / CHANNEL_0..3)
#define MOTOR_LEDC_TIMER   LEDC_TIMER_1
#define MOTOR_LEDC_CHANNEL LEDC_CHANNEL_4

static bool _motor_enabled = false;
static float _current_speed = 0.0f;
static bool _esc_armed = false;
static esp_timer_handle_t _esc_arm_timer = nullptr;

static uint32_t _esc_pulse_to_duty(uint32_t pulse_us)
{
    const uint32_t period_us = (uint32_t)(1000000UL / ESC_PWM_FREQ_HZ);
    const uint32_t duty_max = (1UL << (uint32_t)ESC_PWM_RES) - 1UL;
    if (pulse_us > period_us) pulse_us = period_us;
    return (pulse_us * duty_max) / period_us;
}

static uint32_t _esc_speed_to_duty(float speed)
{
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    const uint32_t pulse_us =
        (uint32_t)(ESC_PULSE_MIN_US + (ESC_PULSE_MAX_US - ESC_PULSE_MIN_US) * speed);
    return _esc_pulse_to_duty(pulse_us);
}

static void _esc_apply_duty(uint32_t duty)
{
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL, duty);
    if (ret != ESP_OK) ESP_LOGE(TAG, "ledc_set_duty failed: %s", esp_err_to_name(ret));
    ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, MOTOR_LEDC_CHANNEL);
    if (ret != ESP_OK) ESP_LOGE(TAG, "ledc_update_duty failed: %s", esp_err_to_name(ret));
}

static void _esc_arm_timer_cb(void* /*arg*/)
{
    if (!_motor_enabled) return;
    _esc_armed = true;
    _esc_apply_duty(_esc_speed_to_duty(_current_speed));
    ESP_LOGI(TAG, "ESC armed; applied target speed %.2f", _current_speed);
}

void HAL_PocketFan::_motor_init()
{
    ESP_LOGI(TAG, "Motor/ESC Init: PWM GPIO%d @ %dHz (1-2ms), MOS_EN GPIO%d",
             MOTOR_PIN, ESC_PWM_FREQ_HZ, (int)MOTOR_MOS_EN_PIN);

    // MOS switch for the motor/ESC power path. Keep it OFF by default to avoid
    // accidental spin at boot.
    gpio_reset_pin(MOTOR_MOS_EN_PIN);
    gpio_config_t mos_conf = {};
    mos_conf.intr_type = GPIO_INTR_DISABLE;
    mos_conf.mode = GPIO_MODE_OUTPUT;
    mos_conf.pin_bit_mask = (1ULL << MOTOR_MOS_EN_PIN);
    mos_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    mos_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    (void)gpio_config(&mos_conf);
    (void)gpio_set_level(MOTOR_MOS_EN_PIN, 0);

    // Reset pin to default state
    gpio_reset_pin((gpio_num_t)MOTOR_PIN);

    // Timer Config
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .duty_resolution  = ESC_PWM_RES,
        .timer_num        = MOTOR_LEDC_TIMER,
        .freq_hz          = ESC_PWM_FREQ_HZ,
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
        .timer_sel      = MOTOR_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
        .flags          = { .output_invert = 0 },
    };
    ret = ledc_channel_config(&ledc_channel);
    if (ret != ESP_OK) ESP_LOGE(TAG, "LEDC Channel Config Failed: %s", esp_err_to_name(ret));

    if (!_esc_arm_timer) {
        const esp_timer_create_args_t timer_args = {
            .callback = &_esc_arm_timer_cb,
            .arg = nullptr,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "esc_arm",
            .skip_unhandled_events = false,
        };
        ret = esp_timer_create(&timer_args, &_esc_arm_timer);
        if (ret != ESP_OK || !_esc_arm_timer) {
            ESP_LOGE(TAG, "esp_timer_create failed: %s", esp_err_to_name(ret));
            _esc_arm_timer = nullptr;
        }
    }
}

void HAL_PocketFan::setFanState(bool enable)
{
    if (enable == _motor_enabled) {
        return;
    }
    _motor_enabled = enable;
    ESP_LOGI(TAG, "Fan State: %s", enable ? "ON" : "OFF");
    if (enable) {
        // Enable MOS power switch before driving PWM.
        (void)gpio_set_level(MOTOR_MOS_EN_PIN, 1);
        _esc_armed = false;

        // Hold minimum throttle while the ESC powers up and arms.
        _esc_apply_duty(_esc_pulse_to_duty(ESC_PULSE_MIN_US));

        if (_esc_arm_timer) {
            (void)esp_timer_stop(_esc_arm_timer);
            const uint64_t delay_us =
                (uint64_t)(ESC_POWERUP_DELAY_MS + ESC_ARM_DELAY_MS) * 1000ULL;
            esp_err_t ret = esp_timer_start_once(_esc_arm_timer, delay_us);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "esp_timer_start_once failed: %s", esp_err_to_name(ret));
                // Fallback: apply immediately.
                _esc_armed = true;
                _esc_apply_duty(_esc_speed_to_duty(_current_speed));
            }
        } else {
            // No timer available; apply immediately.
            _esc_armed = true;
            _esc_apply_duty(_esc_speed_to_duty(_current_speed));
        }
    } else {
        if (_esc_arm_timer) (void)esp_timer_stop(_esc_arm_timer);
        _esc_armed = false;

        // Stop PWM output before cutting power.
        _esc_apply_duty(0);

        // Cut MOS power switch after PWM is stopped.
        (void)gpio_set_level(MOTOR_MOS_EN_PIN, 0);
    }
}

void HAL_PocketFan::setFanSpeed(float speed)
{
    if (speed < 0.0f) speed = 0.0f;
    if (speed > 1.0f) speed = 1.0f;
    _current_speed = speed;

    if (_motor_enabled) {
        // During the arming window we keep minimum throttle; once armed, apply immediately.
        if (_esc_armed) {
            uint32_t duty = _esc_speed_to_duty(speed);
            ESP_LOGI(TAG, "Set Fan Speed: %.2f (ESC duty: %lu)", speed, duty);
            _esc_apply_duty(duty);
        } else {
            ESP_LOGI(TAG, "Set Fan Speed: %.2f (pending ESC arm)", speed);
        }
    }
}
