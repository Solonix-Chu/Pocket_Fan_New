/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <cstring>
#include <esp_timer.h>
#include <LovyanGFX.hpp> // Include LovyanGFX for I2C
#include <nvs_flash.h>
#include <nvs.h>

static const char *TAG = "HAL_PM";

static lgfx::Bus_I2C _ina_bus;
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

// INA219 Registers
#define INA219_REG_CONFIG       0x00
#define INA219_REG_SHUNT        0x01
#define INA219_REG_BUS          0x02
#define INA219_REG_POWER        0x03
#define INA219_REG_CURRENT      0x04
#define INA219_REG_CALIBRATION  0x05

static void writeReg(uint8_t reg, uint16_t val) {
    uint8_t data[3];
    data[0] = reg;
    data[1] = (val >> 8) & 0xFF;
    data[2] = val & 0xFF;
    
    _ina_bus.beginTransaction();
    _ina_bus.writeBytes(data, 3, true, false);
    _ina_bus.endTransaction();
}

static uint16_t readReg(uint8_t reg) {
    uint8_t buf[2] = {0, 0};
    
    // Transaction 1: Set Register Pointer
    _ina_bus.beginTransaction();
    _ina_bus.writeBytes(&reg, 1, true, false);
    _ina_bus.endTransaction(); // Send STOP
    
    // Transaction 2: Read Data
    _ina_bus.beginTransaction();
    _ina_bus.readBytes(buf, 2, false); // Reads 2 bytes
    _ina_bus.endTransaction();
    
    return (buf[0] << 8) | buf[1];
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

    nvs_set_u32(handle, _nvs_key_mah10, mah10);
    nvs_set_u32(handle, _nvs_key_cycles, cyc);
    nvs_set_u32(handle, _nvs_key_mtrh10, mtrh10);
    nvs_commit(handle);
    nvs_close(handle);
}

static void _power_monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");
    int64_t last_tick_us = esp_timer_get_time();
    
    // Init INA219
    // Calibrate: 0.01 Ohm, 1mA LSB -> Cal = 4096
    writeReg(INA219_REG_CALIBRATION, 4096);
    
    // Config: 32V, 2A-32A range (Gain /8), 12-bit, Continuous
    // 0x399F
    writeReg(INA219_REG_CONFIG, 0x399F);
    
    // Initialize data
    memset(&_pm_data_daemon, 0, sizeof(POWER_MONITOR::PMData_t));

    while (1) {
        int64_t now_us = esp_timer_get_time();
        float dt_s = (now_us - last_tick_us) / 1000000.0f;
        if (dt_s < 0) dt_s = 0;
        last_tick_us = now_us;

        float v_bus = 0;
        float v_shunt = 0;
        float current = 0;
        float power = 0;
        
        uint16_t raw_bus = readReg(INA219_REG_BUS);
        uint16_t raw_shunt = readReg(INA219_REG_SHUNT);
        uint16_t raw_current = readReg(INA219_REG_CURRENT);
        uint16_t raw_power = readReg(INA219_REG_POWER);
        
        // Convert
        v_bus = (int16_t)((raw_bus >> 3) * 4) / 1000.0f; // V
        v_shunt = (int16_t)raw_shunt * 0.01f / 1000.0f; // V (10uV LSB)
        current = (int16_t)raw_current * 0.001f; // A (1mA LSB)
        power = (int16_t)raw_power * 0.02f; // W (20mW LSB)

        xSemaphoreTake(_pm_data_handle_mutex, portMAX_DELAY);
        _pm_data_daemon.busVoltage = v_bus;
        _pm_data_daemon.shuntVoltage = v_shunt;
        _pm_data_daemon.shuntCurrent = current;
        _pm_data_daemon.busPower = power;
        _pm_data_daemon.time = esp_timer_get_time() / 1000;
        
        if (_pm_data_daemon.shuntCurrent > _pm_data_daemon.currentPeak)
            _pm_data_daemon.currentPeak = _pm_data_daemon.shuntCurrent;
        if (_pm_data_daemon.shuntCurrent < _pm_data_daemon.currentMin)
            _pm_data_daemon.currentMin = _pm_data_daemon.shuntCurrent;

        // Integrate discharge to estimate cycles
        if (_pm_data_daemon.shuntCurrent > 0.0f && _nominal_capacity_mah > 0.0f) {
            float delta_mah = _pm_data_daemon.shuntCurrent * 1000.0f * (dt_s / 3600.0f);
            _total_discharged_mah += delta_mah;
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
        if (_pending_state_save || (now_ms - _last_state_save_ms) > 60000) {
            _pm_save_state();
            _last_state_save_ms = now_ms;
            _pending_state_save = false;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void HAL_PocketFan::_power_monitor_init() {
    ESP_LOGI(TAG, "Init using LovyanGFX I2C Bus");
    
    // Configure Bus
    auto cfg = _ina_bus.config();
    cfg.i2c_port = 0;
    cfg.i2c_addr = 0x40; // INA219 Addr
    cfg.pin_sda = 33;
    cfg.pin_scl = 34;
    cfg.freq_write = 800000;
    cfg.freq_read = 800000;
    cfg.prefix_len = 0; // Disable control byte (Co/DC) for standard I2C device
    _ina_bus.config(cfg);
    _ina_bus.init();
    
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
