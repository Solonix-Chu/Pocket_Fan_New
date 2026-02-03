/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <functional>
#include <vector>
#include <string>
#include <cmath>
#include "lvgl.h"
#include <cstdint>
#include "types.h"
#include "app_button.h"

/**
 * @brief Provide a singleton to abstract hardware methods
 */
class HAL
{
private:
    static HAL* _hal;

public:
    static HAL* Get();
    static bool Check();
    static bool Inject(HAL* hal);
    static void Destroy();

public:
    HAL() : _display(nullptr) {}
    virtual ~HAL() {}

    static std::string Type() { return Get()->type(); }
    virtual std::string type() { return "Base"; }

    static std::string Version() { return Get()->version(); }
    virtual std::string version() { return APP_VERSION; }

    static std::string CompileDate() { return Get()->compileDate(); }
    virtual std::string compileDate() { return __DATE__; }

    virtual void init() {}

    // Callback type for log page render
    using OnLogPageRenderCallback_t = std::function<void(const std::string&)>;

protected:
    lv_display_t* _display;
    time_t _time_buffer;
    POWER_MONITOR::PMData_t _pm_data;
    CONFIG::SystemConfig_t _config;

    /* -------------------------------------------------------------------------- */
    /*                                     OTA                                    */
    /* -------------------------------------------------------------------------- */
public:
    static void StartBleOta() { Get()->startBleOta(); }
    virtual void startBleOta() {}

    static bool StopBleOta() { return Get()->stopBleOta(); }
    virtual bool stopBleOta() { return false; }

    static BLE_OTA::Status GetBleOtaStatus() { return Get()->getBleOtaStatus(); }
    virtual BLE_OTA::Status getBleOtaStatus() { return BLE_OTA::Status{}; }

    /* -------------------------------------------------------------------------- */
    /*                                   Display                                  */
    /* -------------------------------------------------------------------------- */
public:

    static void LGVL_UPDATE() { Get()->lvgl_update(); }
    virtual void lvgl_update() {}

    /**
     * @brief LVGL display handle
     *
     * @return lv_display_t*
     */
    static lv_display_t* GetDisplay() { return Get()->_display; }

    /**
     * @brief Set display brightness/contrast (if supported)
     */
    static void SetDisplayBrightness(uint8_t level) { Get()->setDisplayBrightness(level); }
    virtual void setDisplayBrightness(uint8_t level) { (void)level; }

    /**
     * @brief Set display inversion (if supported)
     */
    static void SetDisplayInvert(bool invert) { Get()->setDisplayInvert(invert); }
    virtual void setDisplayInvert(bool invert) { (void)invert; }

    /**
     * @brief Render fps panel
     *
     */
    static void RenderFpsPanel() { Get()->renderFpsPanel(); }
    virtual void renderFpsPanel();

    /**
     * @brief Pop error message and wait reboot
     *
     * @param msg
     */
    static void PopFatalError(std::string msg) { Get()->popFatalError(msg); }
    virtual void popFatalError(std::string msg);

    /**
     * @brief Pop warning message and wait continue
     *
     * @param msg
     */
    static void PopWarning(std::string msg) { Get()->popWarning(msg); }
    virtual void popWarning(std::string msg);

    /**
     * @brief Pop success message and wait continue
     *
     * @param msg
     */
    static void PopSuccess(std::string msg, bool showSuccessLabel = true) { Get()->popSuccess(msg, showSuccessLabel); }
    virtual void popSuccess(std::string msg, bool showSuccessLabel = true);

    /* -------------------------------------------------------------------------- */
    /*                                   System                                   */
    /* -------------------------------------------------------------------------- */
public:
    static void Delay(unsigned long milliseconds) { Get()->delay(milliseconds); }
    virtual void delay(unsigned long milliseconds);

    static unsigned long Millis() { return Get()->millis(); }
    virtual unsigned long millis();

    static void PowerOff() { Get()->powerOff(); }
    virtual void powerOff() {}

    static void Reboot() { Get()->reboot(); }
    virtual void reboot() {}

    static void SetSystemTime(tm dateTime) { return Get()->setSystemTime(dateTime); }
    virtual void setSystemTime(tm dateTime) {}

    static tm* GetLocalTime() { return Get()->getLocalTime(); }
    virtual tm* getLocalTime();

    static void FeedTheDog() { Get()->feedTheDog(); }
    virtual void feedTheDog() {}

    static void FactoryReset(OnLogPageRenderCallback_t onLogPageRender) { Get()->factoryReset(onLogPageRender); }
    virtual void factoryReset(OnLogPageRenderCallback_t onLogPageRender) {}

    /* -------------------------------------------------------------------------- */
    /*                                Startup image                               */
    /* -------------------------------------------------------------------------- */
public:
    static bool RenderCustomStartupImage() { return Get()->renderCustomStartupImage(); }
    virtual bool renderCustomStartupImage() { return false; }

    static std::vector<std::string> GetStartupImageList() { return Get()->getStartupImageList(); }
    virtual std::vector<std::string> getStartupImageList() { return std::vector<std::string>(); }

    /* -------------------------------------------------------------------------- */
    /*                                System config                               */
    /* -------------------------------------------------------------------------- */
public:
    static void LoadSystemConfig() { Get()->loadSystemConfig(); }
    virtual void loadSystemConfig() {}

    static void SaveSystemConfig() { Get()->saveSystemConfig(); }
    virtual void saveSystemConfig() {}

    static CONFIG::SystemConfig_t& GetSystemConfig() { return Get()->_config; }
    static void SetSystemConfig(CONFIG::SystemConfig_t cfg) { Get()->_config = cfg; }

    static void ApplySystemConfig() { Get()->applySystemConfig(); }
    virtual void applySystemConfig() {}

    /* -------------------------------------------------------------------------- */
    /*                                   Button                                  */
    /* -------------------------------------------------------------------------- */
public:
    static app_button_state_t GetButton(BUTTON::Button_t button) { return Get()->getButton(button); }
    virtual app_button_state_t getButton(BUTTON::Button_t button) { return APP_BUTTON_STATE_NOCHANGE; }

    static void AllButtonRefresh() {Get()->allButton_refresh();}
    virtual void allButton_refresh() {}

    /* -------------------------------------------------------------------------- */
    /*                                   Encoder                                  */
    /* -------------------------------------------------------------------------- */
public:
    static int GetEncoderCount() { return Get()->getEncoderCount(); }
    virtual int getEncoderCount() { return 0; }

    static void ResetEncoderCount(int value = 0) { Get()->resetEncoderCount(value); }
    virtual void resetEncoderCount(int value) {}

    /* -------------------------------------------------------------------------- */
    /*                                  Touchpad                                  */
    /* -------------------------------------------------------------------------- */
public:
    static void UpdateTouch() { Get()->updateTouch(); }
    virtual void updateTouch() {}

    static bool IsTouching() { return Get()->isTouching(); }
    virtual bool isTouching() { return false; }

    static lv_point_t GetTouchPoint() { return Get()->getTouchPoint(); }
    virtual lv_point_t getTouchPoint() { return {-1, -1}; }

    /* -------------------------------------------------------------------------- */
    /*                                Power monitor                               */
    /* -------------------------------------------------------------------------- */
public:
    static void UpdatePowerMonitor() { Get()->updatePowerMonitor(); }
    virtual void updatePowerMonitor() {}

    static float GetBatteryCycles() { return Get()->getBatteryCycles(); }
    static float GetBatteryDischargedMah() { return Get()->getBatteryDischargedMah(); }
    virtual float getBatteryCycles() { return 0.0f; }
    virtual float getBatteryDischargedMah() { return 0.0f; }
    static float GetMotorHours() { return Get()->getMotorHours(); }
    virtual float getMotorHours() { return 0.0f; }

    static const POWER_MONITOR::PMData_t& GetPowerMonitorData() { return Get()->getPowerMonitorData(); }
    virtual const POWER_MONITOR::PMData_t& getPowerMonitorData() { return _pm_data; }

    static void ResetPowerMonitorData() { Get()->resetPowerMonitorData(); }
    virtual void resetPowerMonitorData() {}

    static bool IsPowerMonitorInLowCurrentMode() { return Get()->isPowerMonitorInLowCurrentMode(); }
    virtual bool isPowerMonitorInLowCurrentMode() { return true; }

    static void PowerMonitorCalibration(const float& currentOffset) { Get()->powerMonitorCalibration(currentOffset); }
    virtual void powerMonitorCalibration(const float& currentOffset) {}

    static POWER_MONITOR::UnitAdaptatedData_t GetUnitAdaptatedVoltage(const float& voltage);
    static POWER_MONITOR::UnitAdaptatedData_t GetUnitAdaptatedCurrent(const float& current);
    static POWER_MONITOR::UnitAdaptatedData_t GetUnitAdaptatedPower(const float& power);
    static POWER_MONITOR::UnitAdaptatedData_t GetUnitAdaptatedCapacity(const float& capacity);
    static POWER_MONITOR::UnitAdaptatedData_t GetUnitAdaptatedEnergy(const float& energy);

    /* -------------------------------------------------------------------------- */
    /*                                     Fan                                    */
    /* -------------------------------------------------------------------------- */
public:
    static void SetFanSpeed(float speed) { Get()->setFanSpeed(speed); }
    virtual void setFanSpeed(float speed) {}

    static void SetFanState(bool enable) { Get()->setFanState(enable); }
    virtual void setFanState(bool enable) {}

    /* -------------------------------------------------------------------------- */
    /*                                     NTC                                    */
    /* -------------------------------------------------------------------------- */
public:
    static float GetNTC(int channel) { return Get()->getNTC(channel); }
    virtual float getNTC(int channel) { return 0.0f; }

    /* -------------------------------------------------------------------------- */
    /*                                     LED                                    */
    /* -------------------------------------------------------------------------- */
public:
    static void SetLed(uint8_t r, uint8_t g, uint8_t b, uint8_t w) { Get()->setLed(r, g, b, w); }
    virtual void setLed(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {}

    static void SetLedBreath(bool enable) { Get()->setLedBreath(enable); }
    virtual void setLedBreath(bool enable) {}

    /* -------------------------------------------------------------------------- */
    /*                                    LVGL                                    */
    /* -------------------------------------------------------------------------- */
public:
    static void LvglLock() { Get()->lvgl_lock(); }
    virtual void lvgl_lock() {}

    static void LvglUnlock() { Get()->lvgl_unlock(); }
    virtual void lvgl_unlock() {}
};
