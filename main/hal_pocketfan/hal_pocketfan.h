/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <hal/hal.h>
// #include "components/hal_lvgl.hpp"

class HAL_PocketFan : public HAL
{
private:
    void _disp_init();
    void _lvgl_init();
    void _button_init();
    void _watch_dog_init();
    void _power_init();
    void _power_monitor_init();
    void _ntc_init();
    void _motor_init();
    void _led_init();

public:
    inline std::string type() override { return "VAMeter"; }
    std::string version() override;

    inline void init() override
    {
        loadSystemConfig();
        _power_init();
        _power_monitor_init();
        _ntc_init();
        _motor_init();
        _led_init();
        _watch_dog_init();
        _disp_init();
        applySystemConfig();
        _lvgl_init();
        _button_init();
    }

    /* -------------------------------------------------------------------------- */
    /*                             Public api override                            */
    /* -------------------------------------------------------------------------- */
    void reboot() override;
    void powerOff() override;

    void feedTheDog() override;

    void setFanSpeed(float speed) override;
    void setFanState(bool enable) override;

    app_button_state_t getButton(BUTTON::Button_t button) override;
    bool isPowerKeyPressed() override;

    void allButton_refresh() override; // Specific to this HAL implementation

    void lvgl_update() override;
    void lvgl_lock() override;
    void lvgl_unlock() override;

    void setDisplayBrightness(uint8_t level) override;
    void setDisplayInvert(bool invert) override;

    // void beep(float frequency, uint32_t duration) override;
    // void beepStop() override;

    // int getEncoderCount() override;
    // void resetEncoderCount(int value) override;

    void updatePowerMonitor() override;
    float getBatteryCycles() override;
    float getBatteryDischargedMah() override;
    float getMotorHours() override;
    void resetPowerMonitorData() override;
    bool isPowerMonitorInLowCurrentMode() override;
    void powerMonitorCalibration(const float& currentOffset) override;

    float getNTC(int channel) override;

    void setLed(uint8_t r, uint8_t g, uint8_t b, uint8_t w) override;
    void setLedBreath(bool enable) override;

    void startBleOta() override;
    bool stopBleOta() override;
    BLE_OTA::Status getBleOtaStatus() override;

    // void setBaseRelay(bool state) override;
    // bool getBaseRelayState() override;

    void loadSystemConfig() override;
    void saveSystemConfig() override;
    void applySystemConfig() override;
    // void startMscMode() override;
    // void stopMscMode() override;
    // void factoryReset(OnLogPageRenderCallback_t onLogPageRender) override;

    // bool creatVaRecorder(VA_RECORDER::TriggerBase* trigger) override;
    // bool isVaRecorderExist() override;
    // bool isVaRecorderRecording() override;
    // bool isVaRecorderSaving() override;
    // bool destroyVaRecorder() override;

    // std::vector<std::string> getVaRecordNameList() override;
    // std::string getLatestVaRecordName() override;
    // VA_RECORDER::Record getVaRecord(const std::string& recordName) override;
    // bool deleteVaRecord(const std::string& recordName) override;

    // bool connectWifi(OnLogPageRenderCallback_t onLogPageRender, bool reconnect) override;
    // bool disconnectWifi() override;
    // uint8_t getApStaNum() override;

    // OTA_UPGRADE::OtaInfo_t getLatestFirmwareInfoViaOta(OnLogPageRenderCallback_t onLogPageRender) override;
    // bool upgradeFirmwareViaOta(OnLogPageRenderCallback_t onLogPageRender, const std::string& firmwareUrl) override;

    // bool uploadVaRecordViaEzData(const std::string& recordName, OnLogPageRenderCallback_t onLogPageRender) override;
    // std::string getVaRecordEzDataUrl() override;

    // bool renderCustomStartupImage() override;
    // std::vector<std::string> getStartupImageList() override;

    // bool startWebServer(OnLogPageRenderCallback_t onLogPageRender, bool autoWifiMode) override;
    // bool stopWebServer() override;
    // std::string getSystemConfigUrl() override;

    // void baseGroveStartTest() override;
    // void baseGroveStopTest() override;
    // bool baseGroveGetIoALevel() override;
    // bool baseGroveGetIoBLevel() override;

    // bool nvsSet(const char* key, const int32_t& value) override;
    // int32_t nvsGet(const char* key) override;
};
