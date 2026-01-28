/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_err.h>
#include <esp_log.h>
#include <cstring>
#include <esp_timer.h>
#include "driver/i2c_master.h"
#include "hal_i2c.h"
#include <nvs_flash.h>
#include <nvs.h>
#include <math.h>

static const char *TAG = "HAL_PM";

static i2c_master_dev_handle_t s_ina226_dev = nullptr;
static i2c_master_dev_handle_t s_ip2369_dev = nullptr;
static SemaphoreHandle_t _pm_data_handle_mutex = NULL;
static POWER_MONITOR::PMData_t _pm_data_daemon;
static bool _is_initialized = false;
static float _total_discharged_mah = 0.0f;
static float _battery_cycles = 0.0f;
static float _motor_hours = 0.0f;
static uint32_t _last_state_save_ms = 0;
static bool _pending_state_save = false;

static constexpr float _nominal_capacity_mah = 2000.0f; // TODO: adjust if real battery capacity differs
static constexpr const char* _nvs_namespace = "pm";
static constexpr const char* _nvs_key_mah10 = "mah10";
static constexpr const char* _nvs_key_cycles = "cycles";
static constexpr const char* _nvs_key_mtrh10 = "mtrh10";
static constexpr float _motor_power_threshold_w = 20.0f;
static constexpr float _motor_hours_cap = 10000.0f;

static constexpr uint8_t INA226_I2C_ADDR = 0x40;
static constexpr uint8_t IP2369_I2C_ADDR = 0x75;
static constexpr uint32_t I2C_FREQ_HZ = 200000;
static constexpr uint32_t I2C_TIMEOUT_MS = 20;
static constexpr float INA226_SHUNT_OHMS = 0.01f;
static constexpr uint16_t INA219_CONFIG_DEFAULT = 0x399F;
static bool _ina_is_226 = false;

// INA226 Registers
#define INA226_REG_CONFIG       0x00
#define INA226_REG_SHUNT        0x01
#define INA226_REG_BUS          0x02

// IP2369 Registers
#define IP2369_REG_CHARGE_STATUS1 0x31
#define IP2369_REG_BAT_VOLT       0x50
#define IP2369_REG_SYS_VOLT       0x52
#define IP2369_REG_BAT_CURR       0x6E
#define IP2369_REG_SYS_CURR       0x70
#define IP2369_REG_SYS_POWER      0x74

#define IP2369_CHG_STATUS_CHARGING   (1U << 5)
#define IP2369_CHG_STATUS_OUTPUT_EN  (1U << 3)

static constexpr uint16_t INA226_CONFIG_DEFAULT =
    (static_cast<uint16_t>(2U) << 9) | (static_cast<uint16_t>(4U) << 6) |
    (static_cast<uint16_t>(4U) << 3) | static_cast<uint16_t>(0x07);

static bool i2c_write_u16_be(i2c_master_dev_handle_t dev, uint8_t reg, uint16_t val)
{
    if (!dev) {
        return false;
    }
    uint8_t data[3] = {reg, static_cast<uint8_t>((val >> 8) & 0xFF), static_cast<uint8_t>(val & 0xFF)};
    return i2c_master_transmit(dev, data, sizeof(data), I2C_TIMEOUT_MS) == ESP_OK;
}

static bool i2c_read_bytes(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *out, size_t len)
{
    if (!dev || !out || len == 0) {
        return false;
    }
    return i2c_master_transmit_receive(dev, &reg, 1, out, len, I2C_TIMEOUT_MS) == ESP_OK;
}

static bool i2c_read_u16_be(i2c_master_dev_handle_t dev, uint8_t reg, uint16_t *out)
{
    uint8_t buf[2] = {0, 0};
    if (!i2c_read_bytes(dev, reg, buf, sizeof(buf))) {
        return false;
    }
    if (out) {
        *out = static_cast<uint16_t>(buf[0] << 8) | buf[1];
    }
    return true;
}

static bool i2c_read_u16_le(i2c_master_dev_handle_t dev, uint8_t reg, uint16_t *out)
{
    uint8_t buf[2] = {0, 0};
    if (!i2c_read_bytes(dev, reg, buf, sizeof(buf))) {
        return false;
    }
    if (out) {
        *out = static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1]) << 8);
    }
    return true;
}

static bool i2c_read_u8(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *out)
{
    uint8_t buf = 0;
    if (!i2c_read_bytes(dev, reg, &buf, 1)) {
        return false;
    }
    if (out) {
        *out = buf;
    }
    return true;
}

static void _ina_detect_and_config() {
    uint16_t man_id = 0;
    uint16_t die_id = 0;
    bool got_id = i2c_read_u16_be(s_ina226_dev, 0xFE, &man_id) &&
                  i2c_read_u16_be(s_ina226_dev, 0xFF, &die_id);
    _ina_is_226 = got_id && (man_id == 0x5449);
    uint16_t cfg = _ina_is_226 ? INA226_CONFIG_DEFAULT : INA219_CONFIG_DEFAULT;
    i2c_write_u16_be(s_ina226_dev, INA226_REG_CONFIG, cfg);
    if (_ina_is_226) {
        ESP_LOGI(TAG, "INA226 detected (man=0x%04X die=0x%04X)", man_id, die_id);
    } else {
        ESP_LOGW(TAG, "INA226 ID not found, fallback to INA219 (man=0x%04X die=0x%04X)", man_id, die_id);
    }
}

static void _pm_load_state() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    nvs_handle_t handle;
    if (nvs_open(_nvs_namespace, NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed");
        return;
    }

    uint32_t mah10 = 0;
    if (nvs_get_u32(handle, _nvs_key_mah10, &mah10) == ESP_OK) {
        _total_discharged_mah = mah10 / 10.0f;
    }

    uint32_t cyc = 0;
    if (nvs_get_u32(handle, _nvs_key_cycles, &cyc) == ESP_OK) {
        _battery_cycles = (float)cyc;
    }

    uint32_t mtrh10 = 0;
    if (nvs_get_u32(handle, _nvs_key_mtrh10, &mtrh10) == ESP_OK) {
        _motor_hours = mtrh10 / 10.0f;
    }

    nvs_close(handle);
}

static void _pm_save_state() {
    nvs_handle_t handle;
    if (nvs_open(_nvs_namespace, NVS_READWRITE, &handle) != ESP_OK) {
        ESP_LOGW(TAG, "NVS open failed (save)");
        return;
    }

    uint32_t mah10 = (uint32_t)(_total_discharged_mah * 10.0f);
    uint32_t cyc = (uint32_t)(_battery_cycles + 0.5f); // store rounded cycles
    uint32_t mtrh10 = (uint32_t)(_motor_hours * 10.0f);

    esp_err_t err = nvs_set_u32(handle, _nvs_key_mah10, mah10);
    if (err != ESP_OK) ESP_LOGW(TAG, "NVS write mah10 failed: %s", esp_err_to_name(err));

    err = nvs_set_u32(handle, _nvs_key_cycles, cyc);
    if (err != ESP_OK) ESP_LOGW(TAG, "NVS write cycles failed: %s", esp_err_to_name(err));

    err = nvs_set_u32(handle, _nvs_key_mtrh10, mtrh10);
    if (err != ESP_OK) ESP_LOGW(TAG, "NVS write motor hours failed: %s", esp_err_to_name(err));

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "NVS commit failed: %s", esp_err_to_name(err));
    }
    nvs_close(handle);
}

static void _power_monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");
    int64_t last_tick_us = esp_timer_get_time();
    uint32_t last_log_ms = 0;
    
    // Init INA226/INA219
    _ina_detect_and_config();
    
    // Initialize data
    memset(&_pm_data_daemon, 0, sizeof(POWER_MONITOR::PMData_t));

    while (1) {
        int64_t now_us = esp_timer_get_time();
        float dt_s = (now_us - last_tick_us) / 1000000.0f;
        if (dt_s < 0) dt_s = 0;
        last_tick_us = now_us;

        float bus_v = 0.0f;
        float shunt_v = 0.0f;
        float current_a = 0.0f;
        float ina_power_w = 0.0f;
        bool ina_ok = false;

        uint16_t raw_bus = 0;
        uint16_t raw_shunt_u = 0;
        if (i2c_read_u16_be(s_ina226_dev, INA226_REG_BUS, &raw_bus) &&
            i2c_read_u16_be(s_ina226_dev, INA226_REG_SHUNT, &raw_shunt_u)) {
            ina_ok = true;
            int16_t raw_shunt = static_cast<int16_t>(raw_shunt_u);
            uint16_t raw_bus_v = raw_bus;
            float bus_lsb_v = 0.00125f;
            float shunt_lsb_v = 0.0000025f;
            if (!_ina_is_226) {
                raw_bus_v = raw_bus >> 3;
                bus_lsb_v = 0.004f;
                shunt_lsb_v = 0.00001f;
            }
            bus_v = static_cast<float>(raw_bus_v) * bus_lsb_v;
            shunt_v = static_cast<float>(raw_shunt) * shunt_lsb_v;
            if (INA226_SHUNT_OHMS > 0.0f) {
                current_a = shunt_v / INA226_SHUNT_OHMS;
            }
            ina_power_w = bus_v * current_a;
        }

        uint8_t charge_status1 = 0;
        uint16_t vbat_mv = 0;
        uint16_t vsys_mv = 0;
        uint16_t vsys_power_raw = 0;
        int16_t ibat_ma = 0;
        int16_t isys_ma = 0;
        bool ip_ok = true;
        ip_ok = ip_ok && i2c_read_u8(s_ip2369_dev, IP2369_REG_CHARGE_STATUS1, &charge_status1);
        ip_ok = ip_ok && i2c_read_u16_le(s_ip2369_dev, IP2369_REG_BAT_VOLT, &vbat_mv);
        ip_ok = ip_ok && i2c_read_u16_le(s_ip2369_dev, IP2369_REG_SYS_VOLT, &vsys_mv);
        uint16_t ibat_raw = 0;
        uint16_t isys_raw = 0;
        ip_ok = ip_ok && i2c_read_u16_le(s_ip2369_dev, IP2369_REG_BAT_CURR, &ibat_raw);
        ip_ok = ip_ok && i2c_read_u16_le(s_ip2369_dev, IP2369_REG_SYS_CURR, &isys_raw);
        ip_ok = ip_ok && i2c_read_u16_le(s_ip2369_dev, IP2369_REG_SYS_POWER, &vsys_power_raw);

        float vbat_v = 0.0f;
        float vsys_v = 0.0f;
        float ibat_a = 0.0f;
        float isys_a = 0.0f;
        float input_power_w = 0.0f;
        float output_power_w = 0.0f;
        bool charging = false;
        bool output_en = false;

        if (ip_ok) {
            vbat_v = static_cast<float>(vbat_mv) / 1000.0f;
            vsys_v = static_cast<float>(vsys_mv) / 1000.0f;
            ibat_ma = static_cast<int16_t>(ibat_raw);
            isys_ma = static_cast<int16_t>(isys_raw);
            ibat_a = static_cast<float>(ibat_ma) / 1000.0f;
            isys_a = static_cast<float>(isys_ma) / 1000.0f;
            float vsys_power_reg_w = static_cast<float>(vsys_power_raw) * 0.01f;
            float bat_power_w = vbat_v * ibat_a;
            float vsys_power_calc_w = fabsf(vsys_v * isys_a);
            charging = (charge_status1 & IP2369_CHG_STATUS_CHARGING) != 0;
            output_en = (charge_status1 & IP2369_CHG_STATUS_OUTPUT_EN) != 0;

            input_power_w = charging ? fabsf(bat_power_w) : 0.0f;
            output_power_w = (vsys_power_reg_w > 0.001f) ? vsys_power_reg_w : vsys_power_calc_w;
            if (!output_en) {
                output_power_w = 0.0f;
            }
        } else {
            output_power_w = ina_power_w;
        }

        float report_bus_v = ina_ok ? bus_v : (ip_ok ? vsys_v : 0.0f);
        float report_output_w = ip_ok ? output_power_w : ina_power_w;

        xSemaphoreTake(_pm_data_handle_mutex, portMAX_DELAY);
        _pm_data_daemon.busVoltage = report_bus_v;
        _pm_data_daemon.shuntVoltage = shunt_v;
        _pm_data_daemon.shuntCurrent = current_a;
        _pm_data_daemon.busPower = report_output_w;
        _pm_data_daemon.inputPower = input_power_w;
        _pm_data_daemon.outputPower = report_output_w;
        _pm_data_daemon.time = esp_timer_get_time() / 1000;
        
        if (_pm_data_daemon.shuntCurrent > _pm_data_daemon.currentPeak)
            _pm_data_daemon.currentPeak = _pm_data_daemon.shuntCurrent;
        if (_pm_data_daemon.shuntCurrent < _pm_data_daemon.currentMin)
            _pm_data_daemon.currentMin = _pm_data_daemon.shuntCurrent;

        // Integrate discharge to estimate cycles
        if (_pm_data_daemon.shuntCurrent > 0.0f && _nominal_capacity_mah > 0.0f) {
            float delta_mah = _pm_data_daemon.shuntCurrent * 1000.0f * (dt_s / 3600.0f);
            _total_discharged_mah += delta_mah;
            // ESP_LOGW(TAG, "Discharged +%.6f mAh, total %.3f mAh", delta_mah, _total_discharged_mah);
            while (_total_discharged_mah >= _nominal_capacity_mah) {
                _battery_cycles += 1.0f;
                _total_discharged_mah -= _nominal_capacity_mah;
                _pending_state_save = true;
            }
        }

        // Integrate motor runtime when power >= threshold
        if (_pm_data_daemon.busPower >= _motor_power_threshold_w && _motor_hours < _motor_hours_cap) {
            _motor_hours += dt_s / 3600.0f;
            if (_motor_hours > _motor_hours_cap) _motor_hours = _motor_hours_cap;
            _pending_state_save = true;
        }
            
        xSemaphoreGive(_pm_data_handle_mutex);

        uint32_t now_ms = (uint32_t)(now_us / 1000);
        if ((now_ms - last_log_ms) >= 1000) {
            last_log_ms = now_ms;
            if (ip_ok) {
                const char* mode_str = "IDLE";
                if (charging && output_en) mode_str = "DUAL";
                else if (charging) mode_str = "IN";
                else if (output_en) mode_str = "OUT";
                ESP_LOGI(TAG,
                         "IP2369[%s]: VBAT=%.3fV IBAT=%.3fA Pin=%.2fW VSYS=%.3fV ISYS=%.3fA Pout=%.2fW",
                         mode_str, vbat_v, ibat_a, input_power_w, vsys_v, isys_a, output_power_w);
            } else {
                ESP_LOGW(TAG, "IP2369 read failed");
            }

            if (ina_ok) {
                ESP_LOGI(TAG, "INA226: V=%.3fV I=%.3fA P=%.2fW", bus_v, current_a, ina_power_w);
            } else {
                ESP_LOGW(TAG, "INA226 read failed");
            }
        }
        if (_pending_state_save || (now_ms - _last_state_save_ms) > 60000) {
            _pm_save_state();
            _last_state_save_ms = now_ms;
            _pending_state_save = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void HAL_PocketFan::_power_monitor_init() {
    ESP_LOGI(TAG, "Init power monitor I2C");

    hal_pocketfan_i2c_init();
    i2c_master_bus_handle_t bus = hal_pocketfan_i2c_get();
    if (!bus) {
        ESP_LOGE(TAG, "I2C bus not ready");
        return;
    }

    if (!s_ina226_dev) {
        i2c_device_config_t ina_cfg = {};
        ina_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        ina_cfg.device_address = INA226_I2C_ADDR;
        ina_cfg.scl_speed_hz = I2C_FREQ_HZ;
        ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &ina_cfg, &s_ina226_dev));
    }

    if (!s_ip2369_dev) {
        i2c_device_config_t ip_cfg = {};
        ip_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        ip_cfg.device_address = IP2369_I2C_ADDR;
        ip_cfg.scl_speed_hz = I2C_FREQ_HZ;
        ESP_ERROR_CHECK(i2c_master_bus_add_device(bus, &ip_cfg, &s_ip2369_dev));
    }
    
    _pm_data_handle_mutex = xSemaphoreCreateMutex();
    _pm_load_state();
    // Start task
    xTaskCreate(_power_monitor_task, "pm_task", 4096, NULL, 5, NULL);
    _is_initialized = true;
}

void HAL_PocketFan::updatePowerMonitor() {
    if (!_is_initialized) return;
    xSemaphoreTake(_pm_data_handle_mutex, portMAX_DELAY);
    _pm_data = _pm_data_daemon;
    xSemaphoreGive(_pm_data_handle_mutex);
}

void HAL_PocketFan::resetPowerMonitorData() {
    xSemaphoreTake(_pm_data_handle_mutex, portMAX_DELAY);
    _pm_data_daemon.currentPeak = -999.0f;
    _pm_data_daemon.currentMin = 999.0f;
    xSemaphoreGive(_pm_data_handle_mutex);
}

bool HAL_PocketFan::isPowerMonitorInLowCurrentMode() { 
    return false; // Not supported
}

void HAL_PocketFan::powerMonitorCalibration(const float& currentOffset) {
    // Implement offset if needed
}

float HAL_PocketFan::getBatteryCycles() {
    return _battery_cycles;
}

float HAL_PocketFan::getBatteryDischargedMah() {
    return _total_discharged_mah;
}

float HAL_PocketFan::getMotorHours() {
    return _motor_hours;
}
