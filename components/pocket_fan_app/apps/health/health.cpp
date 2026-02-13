#include "health.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include <cstdio>

HealthApp::HealthApp() {
    setAppInfo().name = "Health";
    setAppInfo().icon = nullptr;
}

void HealthApp::onOpen() {
    const auto& tr = AssetPool::GetText();
    _view = new SettingsView();
    _view->addSettingsItem({tr.PocketFan_Health_BatteryCyclesNA, false, false, {}});
    _view->addSettingsItem({tr.PocketFan_Health_BatteryHealthNA, false, false, {}});
    _view->addSettingsItem({tr.PocketFan_Health_MotorHoursNA, false, false, {}});
    _view->addSettingsItem({tr.PocketFan_Health_MotorHealthNA, false, false, {}});
    _view->addSettingsItem({tr.PocketFan_Health_DeviceHealthNA, false, false, {}});
    _view->addSettingsItem({tr.PocketFan_Health_SuggestionNA, false, false, {}});
    _view->addSettingsItem({tr.AppSettings_Option_Back, false, false, [this]() {
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    _view->init();
    _last_ms = HAL::Millis();
    _render();
}

float HealthApp::_calcBatteryHealth() const {
    // Simple linear model: 100% down to a floor of 40% over 1500 cycles.
    const float kMaxCycles = 1500.0f;
    const float kMinHealth = 40.0f;
    float cycles = HAL::GetBatteryCycles();
    float h = 100.0f - (100.0f - kMinHealth) * cycles / kMaxCycles;
    if (h < kMinHealth) h = kMinHealth;
    if (h > 100.0f) h = 100.0f;
    return h;
}

float HealthApp::_calcMotorHealth() const {
    // Degrade 1% every 100h, floor at 40%.
    float motor_hours = HAL::GetMotorHours();
    float h = 100.0f - (motor_hours / 100.0f);
    if (h < 40.0f) h = 40.0f;
    if (h > 100.0f) h = 100.0f;
    return h;
}

void HealthApp::_updateStats(float /*dt_hours*/) {
    // HAL handles motor hour accumulation internally; just refresh power data.
    HAL::UpdatePowerMonitor();
}

void HealthApp::_render() {
    if (!_view) return;

    const auto& tr = AssetPool::GetText();
    char buf[48];

    // Battery cycles
    std::snprintf(buf, sizeof(buf), tr.PocketFan_Health_BatteryCyclesFmt, HAL::GetBatteryCycles());
    _view->updateItemText(0, buf);

    // Battery health
    float bat_h = _calcBatteryHealth();
    std::snprintf(buf, sizeof(buf), tr.PocketFan_Health_BatteryHealthFmt, bat_h);
    _view->updateItemText(1, buf);

    // Motor hours
    float motor_hours = HAL::GetMotorHours();
    if (motor_hours >= 10000.0f) {
        _view->updateItemText(2, tr.PocketFan_Health_MotorHoursOver);
    } else {
        std::snprintf(buf, sizeof(buf), tr.PocketFan_Health_MotorHoursFmt, motor_hours);
        _view->updateItemText(2, buf);
    }

    // Motor health
    float motor_h = _calcMotorHealth();
    std::snprintf(buf, sizeof(buf), tr.PocketFan_Health_MotorHealthFmt, motor_h);
    _view->updateItemText(3, buf);

    // Device health
    float device_h = (bat_h + motor_h) / 2.0f;
    std::snprintf(buf, sizeof(buf), tr.PocketFan_Health_DeviceHealthFmt, device_h);
    _view->updateItemText(4, buf);

    // Suggestion
    if (device_h <= 70.0f) {
        _view->updateItemText(5, tr.PocketFan_Health_SuggestionBad);
    } else {
        _view->updateItemText(5, tr.PocketFan_Health_SuggestionGood);
    }
}

void HealthApp::onRunning() {
    uint32_t now = HAL::Millis();
    float dt = (_last_ms == 0) ? 0.0f : (now - _last_ms) / 1000.0f;
    _last_ms = now;
    if (dt > 0.5f) dt = 0.5f; // clamp to avoid large jumps

    _updateStats(dt / 3600.0f); // convert to hours
    _render();
    if (_view) {
        _view->update();
    }
}

void HealthApp::onClose() {
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}

void HealthApp::onDestroy() {
    onClose();
}
