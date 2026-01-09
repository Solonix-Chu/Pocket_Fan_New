#include "health.h"
#include "../../hal/hal.h"
#include "../apps.h"
#include <cstdio>

HealthApp::HealthApp() {
    setAppInfo().name = "Health";
    setAppInfo().icon = nullptr;
}

void HealthApp::onOpen() {
    _view = new HealthView();
    _view->setItems({
        "Battery Cycles: --",
        "Battery Health: --",
        "Motor Hours: --",
        "Motor Health: --",
        "Device Health: --",
        "Suggestion: --"
    });
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

    char buf[48];

    // Battery cycles
    std::snprintf(buf, sizeof(buf), "Battery Cycles: %.0f", HAL::GetBatteryCycles());
    _view->updateItem(0, buf);

    // Battery health
    float bat_h = _calcBatteryHealth();
    std::snprintf(buf, sizeof(buf), "Battery Health: %.0f%%", bat_h);
    _view->updateItem(1, buf);

    // Motor hours
    float motor_hours = HAL::GetMotorHours();
    if (motor_hours >= 10000.0f) {
        _view->updateItem(2, "Motor Hours: >10000h");
    } else {
        std::snprintf(buf, sizeof(buf), "Motor Hours: %.1fh", motor_hours);
        _view->updateItem(2, buf);
    }

    // Motor health
    float motor_h = _calcMotorHealth();
    std::snprintf(buf, sizeof(buf), "Motor Health: %.0f%%", motor_h);
    _view->updateItem(3, buf);

    // Device health
    float device_h = (bat_h + motor_h) / 2.0f;
    std::snprintf(buf, sizeof(buf), "Device Health: %.0f%%", device_h);
    _view->updateItem(4, buf);

    // Suggestion
    if (device_h <= 70.0f) {
        _view->updateItem(5, "Suggestion: Replace device");
    } else {
        _view->updateItem(5, "Suggestion: Good");
    }
}

void HealthApp::onRunning() {
    // Exit on OK
    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
        return;
    }

    // Scroll list with UP/DOWN
    const float step_px = 16.0f; // roughly one row
    float max_offset = 0.0f;
    if (_view) {
        float content_h = _view->getContentHeight();
        float viewport_h = 64.0f; // screen height
        if (content_h > viewport_h) {
            max_offset = content_h - viewport_h;
        }
    }

    if (HAL::GetButton(BUTTON::BTN_UP) == APP_BUTTON_STATE_CLICKED) {
        if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        _scroll_offset -= step_px;
        if (_scroll_offset < 0.0f) _scroll_offset = 0.0f;
    } else if (HAL::GetButton(BUTTON::BTN_DOWN) == APP_BUTTON_STATE_CLICKED) {
        if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        _scroll_offset += step_px;
        if (_scroll_offset > max_offset) _scroll_offset = max_offset;
    }

    uint32_t now = HAL::Millis();
    float dt = (_last_ms == 0) ? 0.0f : (now - _last_ms) / 1000.0f;
    _last_ms = now;
    if (dt > 0.5f) dt = 0.5f; // clamp to avoid large jumps

    _updateStats(dt / 3600.0f); // convert to hours
    _render();
    if (_view) {
        _view->setScrollOffset(_scroll_offset);
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
