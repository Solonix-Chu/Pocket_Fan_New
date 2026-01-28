#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INA219_DEFAULT_I2C_ADDR 0x40
#define INA219_DEFAULT_SCL_HZ   200000
#define INA219_DEFAULT_SHUNT_OHMS 0.01f
#define INA219_DEFAULT_CONFIG 0x399F

typedef struct {
    i2c_master_dev_handle_t dev_handle;
    float shunt_resistance_ohms;
} ina219_handle_t;

typedef struct {
    float bus_v;
    float shunt_mv;
    float current_a;
    float power_w;
} ina219_measurements_t;

typedef struct {
    uint16_t i2c_addr;
    uint32_t scl_speed_hz;
    float shunt_resistance_ohms;
    uint16_t config_reg;
} ina219_config_t;

#define INA219_CONFIG_DEFAULT() { \
    .i2c_addr = INA219_DEFAULT_I2C_ADDR, \
    .scl_speed_hz = INA219_DEFAULT_SCL_HZ, \
    .shunt_resistance_ohms = INA219_DEFAULT_SHUNT_OHMS, \
    .config_reg = INA219_DEFAULT_CONFIG, \
}

esp_err_t ina219_init(i2c_master_bus_handle_t bus, const ina219_config_t *config, ina219_handle_t *out_handle);
esp_err_t ina219_read_measurements(ina219_handle_t *handle, ina219_measurements_t *out);

#ifdef __cplusplus
}
#endif
