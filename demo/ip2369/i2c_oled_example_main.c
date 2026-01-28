/*
 * SPDX-FileCopyrightText: 2021-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_err.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "esp_lvgl_port.h"
#include "lvgl_demo_ui.h"
#include "ip2369.h"
#include "ina219.h"
#include "lvgl.h"

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#include "esp_lcd_sh1107.h"
#else
#include "esp_lcd_panel_vendor.h"
#endif

static const char *TAG = "example";

#define I2C_BUS_PORT  0
#define IP2369_POLL_MS 1000
#define INA219_SHUNT_OHMS 0.01f

#define IP2369_CHG_STATUS_CHARGING   (1U << 5)
#define IP2369_CHG_STATUS_OUTPUT_EN  (1U << 3)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////// Please update the following configuration according to your LCD spec //////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define EXAMPLE_PIN_NUM_SDA           33
#define EXAMPLE_PIN_NUM_SCL           34
#define EXAMPLE_PIN_NUM_RST           -1
#define EXAMPLE_I2C_HW_ADDR           0x3C

// The pixel number in horizontal and vertical
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
#define EXAMPLE_LCD_H_RES              128
#define EXAMPLE_LCD_V_RES              CONFIG_EXAMPLE_SSD1306_HEIGHT
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
#define EXAMPLE_LCD_H_RES              64
#define EXAMPLE_LCD_V_RES              128
#endif
// Bit number used to represent command and parameter
#define EXAMPLE_LCD_CMD_BITS           8
#define EXAMPLE_LCD_PARAM_BITS         8

static ip2369_handle_t s_ip2369;
static ina219_handle_t s_ina219;

static void ip2369_task(void *arg)
{
    ip2369_handle_t *handle = (ip2369_handle_t *)arg;
    ip2369_measurements_t meas = {0};
    ina219_measurements_t ina = {0};
    bool ina_valid = false;

    while (true) {
        esp_err_t err = ip2369_read_measurements(handle, &meas);
        if (err == ESP_OK) {
            float vbat_v = (float)meas.vbat_mv / 1000.0f;
            float vsys_v = (float)meas.vsys_mv / 1000.0f;
            float ibat_a = (float)meas.ibat_ma / 1000.0f;
            float isys_a = (float)meas.isys_ma / 1000.0f;
            float vsys_power_reg_w = (float)meas.vsys_power_raw * 0.01f;
            float bat_power_w = vbat_v * ibat_a;
            float vsys_power_calc_w = fabsf(vsys_v * isys_a);
            bool charging = (meas.charge_status1 & IP2369_CHG_STATUS_CHARGING) != 0;
            bool output_en = (meas.charge_status1 & IP2369_CHG_STATUS_OUTPUT_EN) != 0;

            float input_power_w = charging ? fabsf(bat_power_w) : 0.0f;
            float output_power_w = (vsys_power_reg_w > 0.001f) ? vsys_power_reg_w : vsys_power_calc_w;

            lvgl_power_mode_t mode = LVGL_POWER_MODE_IDLE;
            const char *mode_str = "IDLE";
            if (charging && output_en) {
                mode = LVGL_POWER_MODE_DUAL;
                mode_str = "DUAL";
            } else if (charging) {
                mode = LVGL_POWER_MODE_INPUT;
                mode_str = "IN";
            } else if (output_en) {
                mode = LVGL_POWER_MODE_OUTPUT;
                mode_str = "OUT";
            }

            ESP_LOGI(TAG,
                     "IP2369[%s]: VBAT=%.3fV IBAT=%.3fA Pin=%.2fW VSYS=%.3fV ISYS=%.3fA Pout=%.2fW",
                     mode_str, vbat_v, ibat_a, input_power_w, vsys_v, isys_a, output_power_w);

            esp_err_t ina_err = ina219_read_measurements(&s_ina219, &ina);
            if (ina_err == ESP_OK) {
                ina_valid = true;
                ESP_LOGI(TAG, "INA219: V=%.3fV I=%.3fA P=%.2fW",
                         ina.bus_v, ina.current_a, ina.power_w);
            } else {
                ina_valid = false;
                ESP_LOGW(TAG, "INA219 read failed: %s", esp_err_to_name(ina_err));
            }

            if (lvgl_port_lock(0)) {
                example_lvgl_update_power(mode,
                                          input_power_w,
                                          output_power_w,
                                          vbat_v,
                                          vsys_v,
                                          ina_valid,
                                          ina.bus_v,
                                          ina.current_a,
                                          ina.power_w);
                lvgl_port_unlock();
            }
        } else {
            ESP_LOGW(TAG, "IP2369 read failed: %s", esp_err_to_name(err));
        }

        vTaskDelay(pdMS_TO_TICKS(IP2369_POLL_MS));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Initialize I2C bus");
    i2c_master_bus_handle_t i2c_bus = NULL;
    i2c_master_bus_config_t bus_config = {
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .i2c_port = I2C_BUS_PORT,
        .sda_io_num = EXAMPLE_PIN_NUM_SDA,
        .scl_io_num = EXAMPLE_PIN_NUM_SCL,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus));

    ESP_LOGI(TAG, "Install panel IO");
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = EXAMPLE_I2C_HW_ADDR,
        .scl_speed_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .control_phase_bytes = 1,               // According to SSD1306 datasheet
        .lcd_cmd_bits = EXAMPLE_LCD_CMD_BITS,   // According to SSD1306 datasheet
        .lcd_param_bits = EXAMPLE_LCD_CMD_BITS, // According to SSD1306 datasheet
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
        .dc_bit_offset = 6,                     // According to SSD1306 datasheet
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
        .dc_bit_offset = 0,                     // According to SH1107 datasheet
        .flags =
        {
            .disable_control_phase = 1,
        }
#endif
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_bus, &io_config, &io_handle));

    ESP_LOGI(TAG, "Install SSD1306 panel driver");
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .bits_per_pixel = 1,
        .reset_gpio_num = EXAMPLE_PIN_NUM_RST,
    };
#if CONFIG_EXAMPLE_LCD_CONTROLLER_SSD1306
    esp_lcd_panel_ssd1306_config_t ssd1306_config = {
        .height = EXAMPLE_LCD_V_RES,
    };
    panel_config.vendor_config = &ssd1306_config;
    ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(io_handle, &panel_config, &panel_handle));
#elif CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_new_panel_sh1107(io_handle, &panel_config, &panel_handle));
#endif

    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

#if CONFIG_EXAMPLE_LCD_CONTROLLER_SH1107
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
#endif

    ESP_LOGI(TAG, "Initialize LVGL");
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    lvgl_port_init(&lvgl_cfg);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io_handle,
        .panel_handle = panel_handle,
        .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
        .double_buffer = true,
        .hres = EXAMPLE_LCD_H_RES,
        .vres = EXAMPLE_LCD_V_RES,
        .monochrome = true,
        .rotation = {
            .swap_xy = false,
            .mirror_x = false,
            .mirror_y = false,
        }
    };
    lv_disp_t *disp = lvgl_port_add_disp(&disp_cfg);

    /* Rotation of the screen */
    lv_disp_set_rotation(disp, LV_DISP_ROT_NONE);

    ESP_LOGI(TAG, "Display IP2369 UI");
    // Lock the mutex due to the LVGL APIs are not thread-safe
    if (lvgl_port_lock(0)) {
        example_lvgl_demo_ui(disp);
        // Release the mutex
        lvgl_port_unlock();
    }

    ip2369_config_t ip2369_cfg = IP2369_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(ip2369_init(i2c_bus, &ip2369_cfg, &s_ip2369));

    ina219_config_t ina_cfg = INA219_CONFIG_DEFAULT();
    ina_cfg.shunt_resistance_ohms = INA219_SHUNT_OHMS;
    ESP_ERROR_CHECK(ina219_init(i2c_bus, &ina_cfg, &s_ina219));
    xTaskCreate(ip2369_task, "ip2369_task", 4096, &s_ip2369, 5, NULL);
}
