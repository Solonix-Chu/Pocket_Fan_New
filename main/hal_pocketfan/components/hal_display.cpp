/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#include "../hal_pocketfan.h"
#include <LovyanGFX.hpp>
#include "esp_log.h"

static const char *TAG = "hal_display";

class LGFX_SSD1306 : public lgfx::LGFX_Device
{
    lgfx::Panel_SSD1306     _panel_instance;
    lgfx::Bus_I2C           _bus_instance;

public:
    LGFX_SSD1306(void)
    {
        {
            auto cfg = _bus_instance.config();
            cfg.i2c_port    = 0;
            cfg.freq_write  = 400000;
            cfg.freq_read   = 400000;
            cfg.pin_sda     = 35;
            cfg.pin_scl     = 36;
            cfg.i2c_addr    = 0x3C;
            _bus_instance.config(cfg);
            _panel_instance.setBus(&_bus_instance);
        }

        {
            auto cfg = _panel_instance.config();
            cfg.pin_cs           = -1;
            cfg.pin_rst          = -1;
            cfg.pin_busy         = -1;
            cfg.panel_width      = 128;
            cfg.panel_height     = 64;
            cfg.offset_x         = 0;
            cfg.offset_y         = 0;
            _panel_instance.config(cfg);
        }
        setPanel(&_panel_instance);
    }
};

static LGFX_SSD1306 display; // No longer static, now globally accessible

void HAL_PocketFan::_disp_init()
{
    ESP_LOGI(TAG, "display init");

    display.init();
    display.setBrightness(128);

    _display = &display; // Assign to parent class pointer
    
    // Create a full-screen sprite/canvas
    _canvas = new LGFX_SpriteFx(_display);
    _canvas->createSprite(_display->width(), _display->height());
}