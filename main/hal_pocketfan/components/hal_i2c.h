/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "driver/gpio.h"
#include "driver/i2c_master.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HAL_I2C_PORT_NUM 0
#define HAL_I2C_PIN_SDA  GPIO_NUM_33
#define HAL_I2C_PIN_SCL  GPIO_NUM_34

void hal_pocketfan_i2c_init(void);
i2c_master_bus_handle_t hal_pocketfan_i2c_get(void);

#ifdef __cplusplus
}
#endif
