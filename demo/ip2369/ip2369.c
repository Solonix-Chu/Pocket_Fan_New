#include "ip2369.h"
#include <stddef.h>

#define IP2369_REG_CHARGE_STATUS1 0x31
#define IP2369_REG_BAT_VOLT       0x50
#define IP2369_REG_SYS_VOLT       0x52
#define IP2369_REG_BAT_CURR       0x6E
#define IP2369_REG_SYS_CURR       0x70
#define IP2369_REG_SYS_POWER      0x74

#define IP2369_I2C_TIMEOUT_MS 100

static esp_err_t ip2369_read(ip2369_handle_t *handle, uint8_t reg, uint8_t *out, size_t len)
{
    return i2c_master_transmit_receive(handle->dev_handle, &reg, 1, out, len, IP2369_I2C_TIMEOUT_MS);
}

static esp_err_t ip2369_read_u8(ip2369_handle_t *handle, uint8_t reg, uint8_t *out)
{
    return ip2369_read(handle, reg, out, 1);
}

static esp_err_t ip2369_read_u16(ip2369_handle_t *handle, uint8_t reg, uint16_t *out)
{
    uint8_t buf[2] = {0};
    esp_err_t err = ip2369_read(handle, reg, buf, sizeof(buf));
    if (err != ESP_OK) {
        return err;
    }
    *out = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return ESP_OK;
}

esp_err_t ip2369_init(i2c_master_bus_handle_t bus, const ip2369_config_t *config, ip2369_handle_t *out_handle)
{
    if (bus == NULL || config == NULL || out_handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = config->i2c_addr,
        .scl_speed_hz = config->scl_speed_hz,
    };

    return i2c_master_bus_add_device(bus, &dev_cfg, &out_handle->dev_handle);
}

esp_err_t ip2369_read_measurements(ip2369_handle_t *handle, ip2369_measurements_t *out)
{
    if (handle == NULL || out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ip2369_read_u8(handle, IP2369_REG_CHARGE_STATUS1, &out->charge_status1);
    if (err != ESP_OK) {
        return err;
    }

    uint16_t value = 0;
    err = ip2369_read_u16(handle, IP2369_REG_BAT_VOLT, &value);
    if (err != ESP_OK) {
        return err;
    }
    out->vbat_mv = value;

    err = ip2369_read_u16(handle, IP2369_REG_SYS_VOLT, &value);
    if (err != ESP_OK) {
        return err;
    }
    out->vsys_mv = value;

    err = ip2369_read_u16(handle, IP2369_REG_BAT_CURR, &value);
    if (err != ESP_OK) {
        return err;
    }
    out->ibat_ma = (int16_t)value;

    err = ip2369_read_u16(handle, IP2369_REG_SYS_CURR, &value);
    if (err != ESP_OK) {
        return err;
    }
    out->isys_ma = (int16_t)value;

    err = ip2369_read_u16(handle, IP2369_REG_SYS_POWER, &value);
    if (err != ESP_OK) {
        return err;
    }
    out->vsys_power_raw = value;

    return ESP_OK;
}
