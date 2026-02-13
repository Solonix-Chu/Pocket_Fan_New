#include "detail.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include <cstdio>

namespace {
constexpr const char* kLabelsCn[7] = {
    "实时电量",
    "实时电池电压",
    "实时电池电流",
    "实时设备功率",
    "电机温度",
    "电池温度",
    "主板温度",
};

constexpr const char* kLabelsEn[7] = {
    "Battery Level",
    "Battery Voltage",
    "Battery Current",
    "Device Power",
    "MotorTemp",
    "BatteryTemp",
    "BoardTemp",
};
} // namespace

void DetailApp::onCreate()
{
    setAppInfo().name = "Detail";
}

void DetailApp::onOpen()
{
    _view = new SettingsView();

    for (int i = 0; i < 7; i++) {
        _view->addSettingsItem({_label(i) + ":--", false, false, {}});
    }
    _view->addSettingsItem({AssetPool::GetText().AppSettings_Option_Back, false, false, [this]() {
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    _view->init();
    _render();
}

void DetailApp::onRunning()
{
    _render();
    if (_view) {
        _view->update();
    }
}

void DetailApp::onClose()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}

void DetailApp::onDestroy()
{
    onClose();
}

void DetailApp::_render()
{
    if (!_view) {
        return;
    }

    HAL::UpdatePowerMonitor();
    const auto& pm = HAL::GetPowerMonitorData();
    const bool ip_ok = pm.ip2369_ok;
    const bool has_active_charge_or_discharge =
        ip_ok &&
        pm.ip2369_mode != POWER_MONITOR::IP2369_MODE_IDLE &&
        pm.ip2369_mode != POWER_MONITOR::IP2369_MODE_UNKNOWN;

    char line[64];

    // 0. Battery level
    if (ip_ok) {
        std::snprintf(line, sizeof(line), "%s: %.0f%%", _label(0).c_str(), _calcBatteryPercent(pm.ip2369_vbat));
    } else {
        std::snprintf(line, sizeof(line), "%s: --", _label(0).c_str());
    }
    _view->updateItemText(0, line);

    // 1. Battery voltage
    if (ip_ok) {
        std::snprintf(line, sizeof(line), "%s: %s", _label(1).c_str(), _formatVoltage(pm.ip2369_vbat).c_str());
    } else {
        std::snprintf(line, sizeof(line), "%s: --", _label(1).c_str());
    }
    _view->updateItemText(1, line);

    // 2. Battery current (charging/discharging only)
    if (has_active_charge_or_discharge) {
        std::snprintf(line, sizeof(line), "%s: %s", _label(2).c_str(), _formatCurrent(pm.ip2369_ibat).c_str());
    } else {
        std::snprintf(line, sizeof(line), "%s: --", _label(2).c_str());
    }
    _view->updateItemText(2, line);

    // 3. Device power (charging/discharging only)
    if (has_active_charge_or_discharge) {
        float device_power = pm.ip2369_output_power;
        if (pm.ip2369_mode == POWER_MONITOR::IP2369_MODE_IN) {
            device_power = pm.ip2369_input_power;
        }
        std::snprintf(line, sizeof(line), "%s: %s", _label(3).c_str(), _formatPower(device_power).c_str());
    } else {
        std::snprintf(line, sizeof(line), "%s: --", _label(3).c_str());
    }
    _view->updateItemText(3, line);

    // 4. Motor temperature
    std::snprintf(line, sizeof(line), "%s: %s", _label(4).c_str(), _formatTemp(HAL::GetNTC(0)).c_str());
    _view->updateItemText(4, line);

    // 5. Battery temperature
    if (pm.ip2369_ntc_ok) {
        std::snprintf(line, sizeof(line), "%s: %s", _label(5).c_str(), _formatTemp(pm.ip2369_ntc_temp).c_str());
    } else {
        std::snprintf(line, sizeof(line), "%s: --", _label(5).c_str());
    }
    _view->updateItemText(5, line);

    // 6. Mainboard temperature
    std::snprintf(line, sizeof(line), "%s: %s", _label(6).c_str(), _formatTemp(HAL::GetNTC(1)).c_str());
    _view->updateItemText(6, line);
}

std::string DetailApp::_label(int index) const
{
    if (index < 0 || index >= 7) {
        return "";
    }
    return HAL::GetSystemConfig().localeCode == locale_code_cn ? kLabelsCn[index] : kLabelsEn[index];
}

std::string DetailApp::_formatVoltage(float voltage) const
{
    const auto d = HAL::GetUnitAdaptatedVoltage(voltage);
    return d.value + d.unit;
}

std::string DetailApp::_formatCurrent(float current) const
{
    const auto d = HAL::GetUnitAdaptatedCurrent(current);
    return d.value + d.unit;
}

std::string DetailApp::_formatPower(float power) const
{
    const auto d = HAL::GetUnitAdaptatedPower(power);
    return d.value + d.unit;
}

std::string DetailApp::_formatTemp(float celsius) const
{
    if (celsius < -200.0f || celsius > 200.0f) {
        return "--";
    }
    char buf[20];
    std::snprintf(buf, sizeof(buf), "%.1fC", celsius);
    return buf;
}

float DetailApp::_calcBatteryPercent(float vbat) const
{
    // 4S Li-ion approximation: 12.0V -> 0%, 16.8V -> 100%.
    float pct = (vbat - 12.0f) * 100.0f / (16.8f - 12.0f);
    if (pct < 0.0f) {
        pct = 0.0f;
    }
    if (pct > 100.0f) {
        pct = 100.0f;
    }
    return pct;
}
