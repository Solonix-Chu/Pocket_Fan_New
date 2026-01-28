/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include "hal_display.h"
#include "esp_lvgl_port.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "hal_lvgl";
static bool s_lvgl_port_inited = false;

void HAL_PocketFan::lvgl_update()
{
    if (!s_lvgl_port_inited) {
        lv_timer_handler();
    }
}

void HAL_PocketFan::_lvgl_init()
{
    if (!s_lvgl_port_inited) {
        lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
        esp_err_t err = lvgl_port_init(&lvgl_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "lvgl_port_init failed: %s", esp_err_to_name(err));
            return;
        }
        s_lvgl_port_inited = true;
    }

    esp_lcd_panel_io_handle_t io_handle = hal_display_get_io_handle();
    esp_lcd_panel_handle_t panel_handle = hal_display_get_panel_handle();
    if (!io_handle || !panel_handle) {
        ESP_LOGE(TAG, "LCD panel not initialized");
        return;
    }

    lvgl_port_display_cfg_t disp_cfg = {};
    disp_cfg.io_handle = io_handle;
    disp_cfg.panel_handle = panel_handle;
    disp_cfg.buffer_size = 128 * 64;
    disp_cfg.double_buffer = true;
    disp_cfg.hres = 128;
    disp_cfg.vres = 64;
    disp_cfg.monochrome = true;
    disp_cfg.rotation.swap_xy = false;
    disp_cfg.rotation.mirror_x = false;
    disp_cfg.rotation.mirror_y = false;

    if (!lvgl_port_lock(0)) {
        ESP_LOGE(TAG, "Failed to lock LVGL");
        return;
    }

    _display = lvgl_port_add_disp(&disp_cfg);
    if (!_display) {
        lvgl_port_unlock();
        ESP_LOGE(TAG, "Failed to add LVGL display");
        return;
    }

    lv_display_set_rotation(_display, LV_DISPLAY_ROTATION_0);
    lvgl_port_unlock();
    ESP_LOGI(TAG, "LVGL display ready");
}

void HAL_PocketFan::lvgl_lock()
{
    lvgl_port_lock(0);
}

void HAL_PocketFan::lvgl_unlock()
{
    lvgl_port_unlock();
}
