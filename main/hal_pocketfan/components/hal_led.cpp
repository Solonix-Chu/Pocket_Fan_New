#include "../hal_pocketfan.h"
#include <driver/ledc.h>
#include <esp_timer.h>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define LED_PIN_W GPIO_NUM_35
#define LED_PIN_R GPIO_NUM_36
#define LED_PIN_G GPIO_NUM_37
#define LED_PIN_B GPIO_NUM_38

// LEDC configuration
#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_W        (LED_PIN_W) // Define the output GPIO
#define LEDC_OUTPUT_IO_R        (LED_PIN_R)
#define LEDC_OUTPUT_IO_G        (LED_PIN_G)
#define LEDC_OUTPUT_IO_B        (LED_PIN_B)
#define LEDC_CHANNEL_W          LEDC_CHANNEL_0
#define LEDC_CHANNEL_R          LEDC_CHANNEL_1
#define LEDC_CHANNEL_G          LEDC_CHANNEL_2
#define LEDC_CHANNEL_B          LEDC_CHANNEL_3
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY          (5000) // Frequency in Hertz. Set frequency at 5 kHz

// Breathing control
static bool _breathing_enabled = false;
static esp_timer_handle_t _breath_timer = nullptr;
static uint8_t _target_r = 0, _target_g = 0, _target_b = 0, _target_w = 0;
static float _breath_phase = 0.0f;

static void _ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_DUTY_RES,
        .timer_num        = LEDC_TIMER,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK,
        .deconfigure      = false
    };
    ledc_timer_config(&ledc_timer);

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel[4] = {
        {
            .gpio_num       = LEDC_OUTPUT_IO_W,
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL_W,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER,
            .duty           = 0, // Set duty to 0%
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags          = { .output_invert = 0 }
        },
        {
            .gpio_num       = LEDC_OUTPUT_IO_R,
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL_R,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER,
            .duty           = 0,
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags          = { .output_invert = 0 }
        },
        {
            .gpio_num       = LEDC_OUTPUT_IO_G,
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL_G,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER,
            .duty           = 0,
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags          = { .output_invert = 0 }
        },
        {
            .gpio_num       = LEDC_OUTPUT_IO_B,
            .speed_mode     = LEDC_MODE,
            .channel        = LEDC_CHANNEL_B,
            .intr_type      = LEDC_INTR_DISABLE,
            .timer_sel      = LEDC_TIMER,
            .duty           = 0,
            .hpoint         = 0,
            .sleep_mode     = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD,
            .flags          = { .output_invert = 0 }
        },
    };

    for (int i = 0; i < 4; i++) {
        ledc_channel_config(&ledc_channel[i]);
    }
}

static void _set_duty(ledc_channel_t channel, uint8_t val)
{
    // Map 0-255 to 0-8191 (13 bit)
    uint32_t duty = (uint32_t)val * 8191 / 255;
    ledc_set_duty(LEDC_MODE, channel, duty);
    ledc_update_duty(LEDC_MODE, channel);
}

static void _breath_timer_callback(void* arg)
{
    if (!_breathing_enabled) return;

    _breath_phase += 0.05f;
    if (_breath_phase > 2 * M_PI) _breath_phase -= 2 * M_PI;

    // Sine wave breathing: (sin(x) + 1) / 2 -> 0.0 to 1.0
    float factor = (sin(_breath_phase) + 1.0f) / 2.0f;
    // Gamma correction approximation (square) for better visual perception
    factor = factor * factor; 

    _set_duty(LEDC_CHANNEL_R, (uint8_t)(_target_r * factor));
    _set_duty(LEDC_CHANNEL_G, (uint8_t)(_target_g * factor));
    _set_duty(LEDC_CHANNEL_B, (uint8_t)(_target_b * factor));
    _set_duty(LEDC_CHANNEL_W, (uint8_t)(_target_w * factor));
}

void HAL_PocketFan::_led_init()
{
    _ledc_init();
    
    // Start timer for breathing effect
    const esp_timer_create_args_t timer_args = {
        .callback = &_breath_timer_callback,
        .arg = nullptr,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "led_breath",
        .skip_unhandled_events = false
    };
    esp_timer_create(&timer_args, &_breath_timer);
    esp_timer_start_periodic(_breath_timer, 20000); // 20ms update period (50Hz)
}

void HAL_PocketFan::setLed(uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
    _target_r = r;
    _target_g = g;
    _target_b = b;
    _target_w = w;

    if (!_breathing_enabled) {
        _set_duty(LEDC_CHANNEL_R, r);
        _set_duty(LEDC_CHANNEL_G, g);
        _set_duty(LEDC_CHANNEL_B, b);
        _set_duty(LEDC_CHANNEL_W, w);
    }
}

void HAL_PocketFan::setLedBreath(bool enable)
{
    _breathing_enabled = enable;
    if (!enable) {
        // Restore static color
        setLed(_target_r, _target_g, _target_b, _target_w);
    }
}
