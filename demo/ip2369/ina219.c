#include "ina219.h"
#include <stddef.h>

#define INA219_REG_CONFIG        0x00
#define INA219_REG_SHUNT_VOLTAGE 0x01
#define INA219_REG_BUS_VOLTAGE   0x02

#define INA219_I2C_TIMEOUT_MS 100

static esp_err_t ina219_write_u16(ina219_handle_t *handle, uint8_t reg, uint16_t value)
{
    uint8_t buf[3] = {reg, (uint8_t)(value >> 8), (uint8_t)(value & 0xFF)};
    return i2c_master_transmit(handle->dev_handle, buf, sizeof(buf), INA219_I2C_TIMEOUT_MS);
}

static esp_err_t ina219_read_u16(ina219_handle_t *handle, uint8_t reg, uint16_t *out)
{
    uint8_t buf[2] = {0};
    esp_err_t err = i2c_master_transmit_receive(handle->dev_handle, &reg, 1, buf, sizeof(buf), INA219_I2C_TIMEOUT_MS);
    if (err != ESP_OK) {
        return err;
    }
    *out = (uint16_t)((buf[0] << 8) | buf[1]);
    return ESP_OK;
}

esp_err_t ina219_init(i2c_master_bus_handle_t bus, const ina219_config_t *config, ina219_handle_t *out_handle)
{
    if (bus == NULL || config == NULL || out_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->i2c_addr,
        .scl_speed_hz = config->scl_speed_hz,
    };

    esp_err_t err = i2c_master_bus_add_device(bus, &dev_cfg, &out_handle->dev_handle);
    if (err != ESP_OK) {
        return err;
    }

    out_handle->shunt_resistance_ohms = config->shunt_resistance_ohms;

    return ina219_write_u16(out_handle, INA219_REG_CONFIG, config->config_reg);
}

esp_err_t ina219_read_measurements(ina219_handle_t *handle, ina219_measurements_t *out)
{
    if (handle == NULL || out == NULL || handle->shunt_resistance_ohms <= 0.0f) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t reg = 0;
    esp_err_t err = ina219_read_u16(handle, INA219_REG_SHUNT_VOLTAGE, &reg);
    if (err != ESP_OK) {
        return err;
    }
    int16_t shunt_raw = (int16_t)reg;
    float shunt_uv = (float)shunt_raw * 10.0f;
    out->shunt_mv = shunt_uv / 1000.0f;
    out->current_a = (shunt_uv * 1e-6f) / handle->shunt_resistance_ohms;

    err = ina219_read_u16(handle, INA219_REG_BUS_VOLTAGE, &reg);
    if (err != ESP_OK) {
        return err;
    }
    uint16_t bus_mv = (uint16_t)((reg >> 3) & 0x1FFF) * 4;
    out->bus_v = (float)bus_mv / 1000.0f;
    out->power_w = out->bus_v * out->current_a;

    return ESP_OK;
}
