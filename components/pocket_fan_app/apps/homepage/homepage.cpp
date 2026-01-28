#include "homepage.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include "app_button.h"
#include <esp_log.h>

static const char* TAG = "HomepageApp";

void HomepageApp::onCreate()
{
    setAppInfo().name = "Homepage";
}

void HomepageApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _create_view();
    if (_view) {
        _view->updatePwm(_fan_speed);
        _view->restartEntry();
        _view->tick(HAL::Millis());
    }
}

void HomepageApp::onRunning()
{
    // Update telemetry labels every 200ms
    if (HAL::Millis() - _last_update_time > 200) {
        _last_update_time = HAL::Millis();

        HAL::UpdatePowerMonitor();
        auto pm_data = HAL::GetPowerMonitorData();
        
        auto v = HAL::GetUnitAdaptatedVoltage(pm_data.busVoltage);
        auto a = HAL::GetUnitAdaptatedCurrent(pm_data.shuntCurrent);
        float display_power = pm_data.outputPower;
        if (pm_data.inputPower > 0.01f && pm_data.outputPower < 0.01f) {
            display_power = -pm_data.inputPower;
        }
        auto w = HAL::GetUnitAdaptatedPower(display_power);
        auto cap = HAL::GetUnitAdaptatedCapacity(pm_data.capacity);
        
        float t = HAL::GetNTC(0); // Assuming channel 0 for primary temperature
        char t_buf[32];
        snprintf(t_buf, sizeof(t_buf), "%.1f", t);

        if (_view) {
            _view->updateData(v.value, a.value, w.value);
            _view->updatePage2Data(t_buf, cap.value);
        }
    }

    // Switch between pages using wheel
    if (HAL::GetButton(BUTTON::BTN_RIGHT) == APP_BUTTON_STATE_CLICKED) {
        if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (_current_page == 0) {
            _current_page = 1;
            if (_view) _view->setPage(_current_page);
        }
    } else if (HAL::GetButton(BUTTON::BTN_LEFT) == APP_BUTTON_STATE_CLICKED) {
        if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (_current_page == 1) {
            _current_page = 0;
            if (_view) _view->setPage(_current_page);
        }
    }

    // Transition to Menu on OK button press
    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ESP_LOGI(TAG, "OK button pressed, opening MenuApp");
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }

    // PWM Control using UP/DOWN
    auto btn_up = HAL::GetButton(BUTTON::BTN_UP);
    auto btn_down = HAL::GetButton(BUTTON::BTN_DOWN);
    if (btn_up == APP_BUTTON_STATE_CLICKED || btn_down == APP_BUTTON_STATE_CLICKED) {
        // Calculate step based on scroll speed (time delta)
        uint32_t now = HAL::Millis();
        uint32_t delta = now - _last_scroll_time;
        _last_scroll_time = now;

        int step = 1;
        if (delta < 30) step = 10;
        else if (delta < 50) step = 5;
        else if (delta < 100) step = 2;

        if (btn_up == APP_BUTTON_STATE_CLICKED) {
            // Actually, hal_button.cpp maps BTN_UP to BtnUp.
            if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
            _fan_speed += step;
        } else {
            if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
            _fan_speed -= step;
        }

        // Bound check
        if (_fan_speed > 100) _fan_speed = 100;
        if (_fan_speed < 0) _fan_speed = 0;

        // Apply
        HAL::SetFanState(_fan_speed > 0);
        HAL::SetFanSpeed(_fan_speed / 100.0f);
        if (_view) _view->updatePwm(_fan_speed);
        
        ESP_LOGI(TAG, "PWM updated: %d%% (step: %d, delta: %lu)", _fan_speed, step, delta);
    }

    if (_view) {
        _view->tick(HAL::Millis());
    }
}

void HomepageApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    _destroy_view();
}

void HomepageApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
}

void HomepageApp::_create_view()
{
    if (_view) return;
    _view = new HomepageView();
    _view->init();
}

void HomepageApp::_destroy_view()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}
