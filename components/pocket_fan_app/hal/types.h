/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include <cstdint>
#include <string>
#include <functional>
#include <smooth_ui_toolkit.h>

/* -------------------------------------------------------------------------- */
/*                                       button                               */
/* -------------------------------------------------------------------------- */
namespace BUTTON
{
    enum Button_t
    {
        BTN_UP,
        BTN_DOWN,
        BTN_LEFT,
        BTN_RIGHT,
        BTN_MID,
        BTN_POWER,
        BUTTON_NUM,
    };
} // namespace BUTTON

/* -------------------------------------------------------------------------- */
/*                                Power monitor                               */
/* -------------------------------------------------------------------------- */
namespace POWER_MONITOR
{
    enum Ip2369Mode_t : uint8_t
    {
        IP2369_MODE_IDLE = 0,
        IP2369_MODE_IN = 1,
        IP2369_MODE_OUT = 2,
        IP2369_MODE_DUAL = 3,
        IP2369_MODE_UNKNOWN = 255,
    };

    struct PMData_t
    {
        // Basic
        float busVoltage = 0.0f;
        float busPower = 0.0f;
        float inputPower = 0.0f;
        float outputPower = 0.0f;
        float shuntVoltage = 0.0f;
        float shuntCurrent = 0.0f;

        // From daemon
        float currentAvgSecond = 0.0f;
        float currentAvgMin = 0.0f;
        float currentAvgTotal = 0.0f;
        float currentPeak = 0.0f;
        float currentMin = 0.0f;
        float capacity = 0.0f;
        float energy = 0.0f;
        // char time[12];
        uint32_t time = 0;

        // IP2369 telemetry
        bool ip2369_ok = false;
        Ip2369Mode_t ip2369_mode = IP2369_MODE_UNKNOWN;
        float ip2369_vbat = 0.0f;
        float ip2369_ibat = 0.0f;
        float ip2369_vsys = 0.0f;
        float ip2369_isys = 0.0f;
        float ip2369_input_power = 0.0f;
        float ip2369_output_power = 0.0f;
        bool ip2369_ntc_ok = false;
        float ip2369_ntc_temp = 0.0f;
    };

    struct UnitAdaptatedData_t
    {
        std::string value;
        std::string unit;
    };
}; // namespace POWER_MONITOR

/* -------------------------------------------------------------------------- */
/*                                Localization                                */
/* -------------------------------------------------------------------------- */
enum LocaleCode_t
{
    locale_code_en = 0,
    locale_code_cn,
    locale_code_jp,
};

/* -------------------------------------------------------------------------- */
/*                                System config                               */
/* -------------------------------------------------------------------------- */
namespace CONFIG
{
    // Default config
    struct SystemConfig_t
    {
        int brightness = 255;
        int orientation = 0;
        bool beepOn = true;
        bool highRefreshRate = true;
        bool reverseEncoder = false;
        bool invertDisplay = true;
        bool darkTheme = false;
        LocaleCode_t localeCode = locale_code_en;
        float currentOffset = 0.0f;
        std::string wifiSsid;
        std::string wifiPassword;
        std::string startupImage;
    };

#define PASS_STARTUP_IMAGE_TAG "__pass"
#define DEFAULT_STARTUP_IMAGE_DARK_TAG "__dark"
#define DEFAULT_STARTUP_IMAGE_LIGHT_TAG "__light"
} // namespace CONFIG

/* -------------------------------------------------------------------------- */
/*                                  OTA info                                  */
/* -------------------------------------------------------------------------- */
namespace OTA_UPGRADE
{
    struct OtaInfo_t
    {
        bool getInfoFailed = false;
        bool upgradeAvailable = false;
        bool needUsbFlashing = false;
        std::string latestVersion;
        std::string firmwareUrl;
        std::string description;
    };
}; // namespace OTA_UPGRADE


#define APP_VERSION "V0.0.1"

/* -------------------------------------------------------------------------- */
/*                                  NVS keys                                  */
/* -------------------------------------------------------------------------- */
#define NVS_KEY_APP_HISTORY "app_history"
#define NVS_KEY_BOOT_COUNT "boot_count"
