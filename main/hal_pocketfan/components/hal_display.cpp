/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include "hal_display.h"
#include "hal_i2c.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "hal_display";

static esp_lcd_panel_io_handle_t s_io_handle = nullptr;
static esp_lcd_panel_handle_t s_panel_handle = nullptr;

static constexpr uint32_t SSD1306_PIXEL_CLOCK_HZ = 800 * 1000;
static constexpr int SSD1306_H_RES = 128;
static constexpr int SSD1306_V_RES = 64;
static constexpr int SSD1306_I2C_ADDR = 0x3C;
static constexpr int SSD1306_RST_GPIO = -1;

esp_lcd_panel_io_handle_t hal_display_get_io_handle(void)
{
    return s_io_handle;
}

esp_lcd_panel_handle_t hal_display_get_panel_handle(void)
{
    return s_panel_handle;
}

void HAL_PocketFan::_disp_init()
{
    ESP_LOGI(TAG, "display init");
    ESP_LOGI(TAG, "SSD1306 %dx%d", SSD1306_H_RES, SSD1306_V_RES);

    if (s_panel_handle) {
        ESP_LOGW(TAG, "display already initialized");
        return;
    }

    hal_pocketfan_i2c_init();
    i2c_master_bus_handle_t bus = hal_pocketfan_i2c_get();
    if (!bus) {
        ESP_LOGE(TAG, "I2C bus not ready");
        return;
    }

    esp_lcd_panel_io_i2c_config_t io_config = {};
    io_config.dev_addr = SSD1306_I2C_ADDR;
    io_config.scl_speed_hz = SSD1306_PIXEL_CLOCK_HZ;
    io_config.control_phase_bytes = 1;
    io_config.lcd_cmd_bits = 8;
    io_config.lcd_param_bits = 8;
    io_config.dc_bit_offset = 6;
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(bus, &io_config, &s_io_handle));

    esp_lcd_panel_dev_config_t panel_config = {};
    panel_config.bits_per_pixel = 1;
    panel_config.reset_gpio_num = SSD1306_RST_GPIO;

    esp_lcd_panel_ssd1306_config_t ssd1306_config = {};
    ssd1306_config.height = SSD1306_V_RES;
    panel_config.vendor_config = &ssd1306_config;

    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(s_io_handle, &panel_config, &s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel_handle, true));

    setDisplayInvert(HAL::GetSystemConfig().invertDisplay);
    setDisplayBrightness(static_cast<uint8_t>(HAL::GetSystemConfig().brightness));
}

void HAL_PocketFan::setDisplayBrightness(uint8_t level)
{
    if (!s_io_handle) {
        return;
    }

    uint8_t param = level;
    esp_err_t err = esp_lcd_panel_io_tx_param(s_io_handle, 0x81, &param, 1);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Set contrast failed: %s", esp_err_to_name(err));
    }
}

void HAL_PocketFan::setDisplayInvert(bool invert)
{
    if (!s_panel_handle) {
        return;
    }

    esp_err_t err = esp_lcd_panel_invert_color(s_panel_handle, invert);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Invert display failed: %s", esp_err_to_name(err));
    }
}
