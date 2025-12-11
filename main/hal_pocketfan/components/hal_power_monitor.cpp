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

static const char *TAG = "HAL_PM";

static lgfx::Bus_I2C _ina_bus;
static SemaphoreHandle_t _pm_data_handle_mutex = NULL;
static POWER_MONITOR::PMData_t _pm_data_daemon;
static bool _is_initialized = false;

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

static void _power_monitor_task(void *pvParameters) {
    ESP_LOGI(TAG, "Task started");
    
    // Init INA219
    // Calibrate: 0.01 Ohm, 1mA LSB -> Cal = 4096
    writeReg(INA219_REG_CALIBRATION, 4096);
    
    // Config: 32V, 2A-32A range (Gain /8), 12-bit, Continuous
    // 0x399F
    writeReg(INA219_REG_CONFIG, 0x399F);
    
    // Initialize data
    memset(&_pm_data_daemon, 0, sizeof(POWER_MONITOR::PMData_t));

    while (1) {
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
            
        xSemaphoreGive(_pm_data_handle_mutex);

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