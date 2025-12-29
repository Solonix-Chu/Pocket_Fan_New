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
        auto w = HAL::GetUnitAdaptatedPower(pm_data.busPower);

        if (_view) {
            _view->updateData(v.value + v.unit, a.value + a.unit, w.value + w.unit);
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

    if (_view) {
        _view->update();
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
