#include "homepage.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include "app_button.h"
#include <esp_log.h>
#include <esp_system.h>

static const char* TAG = "HomepageApp";

static void set_random_trackball_targets()
{
    // Random mixed RGBW breathing targets, keep values moderate.
    auto rand_u8 = [](uint8_t max_val) -> uint8_t {
        return (uint8_t)(esp_random() % (max_val + 1));
    };

    uint8_t r = rand_u8(180);
    uint8_t g = rand_u8(180);
    uint8_t b = rand_u8(180);
    uint8_t w = rand_u8(120);

    if ((r | g | b | w) == 0) {
        g = 120;
    }

    HAL::SetLed(r, g, b, w);
}

void HomepageApp::onCreate()
{
    setAppInfo().name = "Homepage";
}

void HomepageApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _last_input_time = HAL::Millis();
    _closing_to_screensaver = false;
    _create_view();
    if (_view) {
        _view->updatePwm(_fan_speed);
        _view->setPage(_current_page);
        _view->restartEntry();
        _view->tick(HAL::Millis());
    }

    // Trackball LED: random mixed breathing in homepage.
    HAL::SetLedBreath(true);
    set_random_trackball_targets();
    _led_last_change_time = HAL::Millis();
}

void HomepageApp::onRunning()
{
    const uint32_t now = HAL::Millis();

    // Inactivity timer (Homepage -> ScreenSaver after 5s without input)
    auto btn_mid = HAL::GetButton(BUTTON::BTN_MID);
    auto btn_left = HAL::GetButton(BUTTON::BTN_LEFT);
    auto btn_right = HAL::GetButton(BUTTON::BTN_RIGHT);
    auto btn_up = HAL::GetButton(BUTTON::BTN_UP);
    auto btn_down = HAL::GetButton(BUTTON::BTN_DOWN);
    auto btn_power = HAL::GetButton(BUTTON::BTN_POWER);

    if (btn_mid != APP_BUTTON_STATE_NOCHANGE ||
        btn_left != APP_BUTTON_STATE_NOCHANGE ||
        btn_right != APP_BUTTON_STATE_NOCHANGE ||
        btn_up != APP_BUTTON_STATE_NOCHANGE ||
        btn_down != APP_BUTTON_STATE_NOCHANGE ||
        btn_power != APP_BUTTON_STATE_NOCHANGE) {
        _last_input_time = now;
    }

    if (now - _last_input_time > 5000) {
        ESP_LOGI(TAG, "Inactivity timeout, opening ScreenSaver");
        _closing_to_screensaver = true;
        mooncake::GetMooncake().openApp(APPS::screensaver_id);
        close();
        return;
    }

    // Update telemetry labels every 200ms
    if (now - _last_update_time > 200) {
        _last_update_time = now;

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

        if (_view) {
            char title_buf[20] = {};
            const char* mode_str = "IDLE";
            bool show_in = false;
            bool show_out = false;
            if (pm_data.ip2369_ok) {
                switch (pm_data.ip2369_mode) {
                case POWER_MONITOR::IP2369_MODE_IN:
                    mode_str = "IN";
                    show_in = true;
                    break;
                case POWER_MONITOR::IP2369_MODE_OUT:
                    mode_str = "OUT";
                    show_out = true;
                    break;
                case POWER_MONITOR::IP2369_MODE_DUAL:
                    mode_str = "DUAL";
                    show_in = true;
                    show_out = true;
                    break;
                case POWER_MONITOR::IP2369_MODE_IDLE:
                default:
                    mode_str = "IDLE";
                    break;
                }
            } else {
                mode_str = "N/A";
            }
            snprintf(title_buf, sizeof(title_buf), "IP2369 %s", mode_str);

            char in_buf[28] = {};
            char out_buf[28] = {};
            char power_buf[28] = {};
            char ntc_buf[20] = {};
            if (pm_data.ip2369_ok && show_in) {
                snprintf(in_buf, sizeof(in_buf), "IN : %4.2fV %+.2fA",
                         pm_data.ip2369_vbat, pm_data.ip2369_ibat);
            } else {
                snprintf(in_buf, sizeof(in_buf), "IN : --.-");
            }

            if (pm_data.ip2369_ok && show_out) {
                snprintf(out_buf, sizeof(out_buf), "OUT: %4.2fV %+.2fA",
                         pm_data.ip2369_vsys, pm_data.ip2369_isys);
            } else {
                snprintf(out_buf, sizeof(out_buf), "OUT: --.-");
            }

            if (pm_data.ip2369_ok && (show_in || show_out)) {
                double pin = show_in ? pm_data.ip2369_input_power : 0.0;
                double pout = show_out ? pm_data.ip2369_output_power : 0.0;
                snprintf(power_buf, sizeof(power_buf), "P: %4.2f/%4.2fW", pin, pout);
            } else {
                snprintf(power_buf, sizeof(power_buf), "P: --.-/--.-W");
            }

            if (pm_data.ip2369_ntc_ok) {
                snprintf(ntc_buf, sizeof(ntc_buf), "NTC: %.1fC", pm_data.ip2369_ntc_temp);
            } else {
                snprintf(ntc_buf, sizeof(ntc_buf), "NTC: --.-C");
            }
            _view->updateIp2369(title_buf, in_buf, out_buf, power_buf, ntc_buf);
        }
    }

    // Switch between pages using wheel
    const int k_last_page = 2;
    if (btn_right == APP_BUTTON_STATE_CLICKED) {
        if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (_current_page < k_last_page) {
            _current_page++;
            if (_view) _view->setPage(_current_page);
        }
    } else if (btn_left == APP_BUTTON_STATE_CLICKED) {
        if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (_current_page > 0) {
            _current_page--;
            if (_view) _view->setPage(_current_page);
        }
    }

    // Transition to Menu on OK button press
    if (btn_mid == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ESP_LOGI(TAG, "OK button pressed, opening MenuApp");
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
        return;
    }

    // PWM Control using UP/DOWN
    if (btn_up == APP_BUTTON_STATE_CLICKED || btn_down == APP_BUTTON_STATE_CLICKED) {
        // Calculate step based on scroll speed (time delta)
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
        _view->tick(now);
    }

    // Change breathing color periodically for "random mix" effect.
    if (now - _led_last_change_time >= 3500) {
        _led_last_change_time = now;
        set_random_trackball_targets();
    }
}

void HomepageApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    if (!_closing_to_screensaver) {
        HAL::SetFanSpeed(0.0f);
        HAL::SetFanState(false);
    }
    _closing_to_screensaver = false;
    _destroy_view();
}

void HomepageApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    HAL::SetFanSpeed(0.0f);
    HAL::SetFanState(false);
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
