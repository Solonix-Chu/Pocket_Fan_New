#pragma once

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

#define IP2369_DEFAULT_I2C_ADDR 0x75
#define IP2369_DEFAULT_SCL_HZ   200000

typedef struct {
    i2c_master_dev_handle_t dev_handle;
} ip2369_handle_t;

typedef struct {
    uint16_t vbat_mv;
    uint16_t vsys_mv;
    int16_t ibat_ma;
    int16_t isys_ma;
    uint16_t vsys_power_raw;
    uint8_t charge_status1;
} ip2369_measurements_t;

typedef struct {
    uint16_t i2c_addr;
    uint32_t scl_speed_hz;
} ip2369_config_t;

#define IP2369_CONFIG_DEFAULT() { \
    .i2c_addr = IP2369_DEFAULT_I2C_ADDR, \
    .scl_speed_hz = IP2369_DEFAULT_SCL_HZ, \
}

esp_err_t ip2369_init(i2c_master_bus_handle_t bus, const ip2369_config_t *config, ip2369_handle_t *out_handle);
esp_err_t ip2369_read_measurements(ip2369_handle_t *handle, ip2369_measurements_t *out);

#ifdef __cplusplus
}
#endif
