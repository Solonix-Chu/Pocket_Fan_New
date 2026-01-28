/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal_i2c.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "hal_i2c";
static i2c_master_bus_handle_t s_i2c_bus = nullptr;

void hal_pocketfan_i2c_init(void)
{
    if (s_i2c_bus) {
        return;
    }

    i2c_master_bus_config_t bus_config = {};
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.i2c_port = HAL_I2C_PORT_NUM;
    bus_config.sda_io_num = HAL_I2C_PIN_SDA;
    bus_config.scl_io_num = HAL_I2C_PIN_SCL;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &s_i2c_bus));
    ESP_LOGI(TAG, "I2C bus initialized");
}

i2c_master_bus_handle_t hal_pocketfan_i2c_get(void)
{
    return s_i2c_bus;
}
