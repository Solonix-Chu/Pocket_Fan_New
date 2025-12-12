/**
 * @file hardware_test.h
 * @brief Hardware test application
 */
#pragma once
#include <mooncake.h>
#include "../../hal/hal.h"
#include <lvgl.h>
#include <string>
#include "esp_log.h"
#include "esp_random.h"

class HardwareTestApp : public mooncake::AppAbility {
private:
    static constexpr const char* TAG = "HardwareTest";
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _label_buttons = nullptr;
    lv_obj_t* _label_power = nullptr;
    lv_obj_t* _label_ntc = nullptr;
    lv_obj_t* _label_fan = nullptr;
    lv_obj_t* _label_led = nullptr;
    lv_obj_t* _label_title = nullptr;
    uint32_t _last_update_time = 0;
    
    int _fan_speed = 50;
    bool _fan_enabled = false;
    int _led_mode = 0; // 0: Off, 1: R, 2: G, 3: B, 4: W, 5: RandBreath

    std::string _last_status = "Press buttons...";

    void _update_led() {
        HAL::SetLedBreath(false); // Disable breathing by default
        switch (_led_mode) {
            case 0: HAL::SetLed(0, 0, 0, 0); break;
            case 1: HAL::SetLed(255, 0, 0, 0); break;
            case 2: HAL::SetLed(0, 255, 0, 0); break;
            case 3: HAL::SetLed(0, 0, 255, 0); break;
            case 4: HAL::SetLed(0, 0, 0, 255); break;
            case 5: 
                HAL::SetLedBreath(true); 
                HAL::SetLed(esp_random()%256, esp_random()%256, esp_random()%256, esp_random()%256);
                break;
        }
    }

    void _update_button_state() {
        if (!_label_buttons) return;

        bool changed = false;
        std::string current_status = "Btn: ";
        bool first = true;
        
        struct BtnInfo {
            BUTTON::Button_t id; 
            const char* name;
        };
        
        BtnInfo btns[] = {
            {BUTTON::BTN_UP, "UP"},
            {BUTTON::BTN_DOWN, "DN"},
            {BUTTON::BTN_LEFT, "LF"},
            {BUTTON::BTN_RIGHT, "RT"},
            {BUTTON::BTN_MID, "OK"}
        };

        for (auto& btn : btns) {
            app_button_state_t state = HAL::GetButton((BUTTON::Button_t)btn.id);
            
            if (state != APP_BUTTON_STATE_NOCHANGE) {
                if (!first) current_status += " | ";
                current_status += std::string(btn.name) + ":";
                switch(state) {
                    case APP_BUTTON_STATE_CLICKED: 
                        current_status += "CLK"; 
                        // Fan Control Logic
                        if (btn.id == BUTTON::BTN_UP) {
                            _fan_speed += 10;
                            if (_fan_speed > 100) _fan_speed = 100;
                            if (_fan_enabled) HAL::SetFanSpeed(_fan_speed / 100.0f);
                        } else if (btn.id == BUTTON::BTN_DOWN) {
                            _fan_speed -= 10;
                            if (_fan_speed < 0) _fan_speed = 0;
                            if (_fan_enabled) HAL::SetFanSpeed(_fan_speed / 100.0f);
                        } else if (btn.id == BUTTON::BTN_MID) {
                            _fan_enabled = !_fan_enabled;
                            HAL::SetFanState(_fan_enabled);
                            if (_fan_enabled) HAL::SetFanSpeed(_fan_speed / 100.0f);
                        } else if (btn.id == BUTTON::BTN_LEFT) {
                            _led_mode = (_led_mode + 1) % 6;
                            _update_led();
                        }
                        break;
                    case APP_BUTTON_STATE_HOLD: current_status += "HLD"; break;
                    case APP_BUTTON_STATE_DOUBLE_CLICK:
                        if (btn.id == BUTTON::BTN_MID) {
                            current_status += "DBL_CLK";
                            HAL::StartBleOta();
                        }
                        break;
                    case APP_BUTTON_STATE_DECIDE_CLICK_COUNT: current_status += "WT"; break;
                    default: current_status += std::to_string((int)state); break;
                }
                // current_status += "\n"; // No newline, try to fit in one line or wrap
                first = false;
                changed = true;
                ESP_LOGI(TAG, "Button event: %s", current_status.c_str());
            }
        }
        
        if (changed) {
            _last_status = current_status;
            lv_label_set_text(_label_buttons, _last_status.c_str());
        }
    }

public:
    HardwareTestApp() {
        setAppInfo().name = "HardwareTest";
        setAppInfo().icon = nullptr;
    }

    void onCreate() override {
        ESP_LOGI(TAG, "onCreate");
        _screen = lv_obj_create(NULL);
        if (!_screen) ESP_LOGE(TAG, "Failed to create screen");
        
        lv_obj_set_style_bg_color(_screen, lv_color_white(), 0); // Force black bg
        lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);

        _label_buttons = lv_label_create(_screen);
        lv_label_set_text(_label_buttons, "Btn: ...");
        lv_obj_set_style_text_color(_label_buttons, lv_color_black(), 0);
        lv_obj_set_style_text_font(_label_buttons, &lv_font_montserrat_10, 0);
        lv_obj_align(_label_buttons, LV_ALIGN_TOP_LEFT, 2, 0);
        lv_label_set_long_mode(_label_buttons, LV_LABEL_LONG_SCROLL); // Scroll if too long
        lv_obj_set_width(_label_buttons, 80); // Limit width to avoid hitting right side

        _label_fan = lv_label_create(_screen);
        lv_label_set_text(_label_fan, "Fan: OFF 50%");
        lv_obj_set_style_text_color(_label_fan, lv_color_black(), 0);
        lv_obj_set_style_text_font(_label_fan, &lv_font_montserrat_10, 0);
        lv_obj_align(_label_fan, LV_ALIGN_TOP_RIGHT, -2, 10);
        lv_obj_set_style_text_align(_label_fan, LV_TEXT_ALIGN_RIGHT, 0);

        _label_led = lv_label_create(_screen);
        lv_label_set_text(_label_led, "LED: OFF");
        lv_obj_set_style_text_color(_label_led, lv_color_black(), 0);
        lv_obj_set_style_text_font(_label_led, &lv_font_montserrat_10, 0);
        lv_obj_align(_label_led, LV_ALIGN_TOP_RIGHT, -2, 20);
        lv_obj_set_style_text_align(_label_led, LV_TEXT_ALIGN_RIGHT, 0);

        _label_ntc = lv_label_create(_screen);
        lv_label_set_text(_label_ntc, "NTC Init...");
        lv_obj_set_style_text_color(_label_ntc, lv_color_black(), 0);
        lv_obj_set_style_text_font(_label_ntc, &lv_font_montserrat_10, 0);
        lv_label_set_long_mode(_label_ntc, LV_LABEL_LONG_WRAP);
        lv_obj_set_style_text_align(_label_ntc, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_align(_label_ntc, LV_ALIGN_BOTTOM_RIGHT, -2, 0);

        _label_power = lv_label_create(_screen);
        lv_label_set_text(_label_power, "Init Power...");
        lv_obj_set_style_text_color(_label_power, lv_color_black(), 0);
        lv_obj_set_style_text_font(_label_power, &lv_font_montserrat_10, 0);
        lv_obj_align(_label_power, LV_ALIGN_BOTTOM_LEFT, 2, 0);
    }

    void onOpen() override {
        ESP_LOGI(TAG, "onOpen");
        if (_screen) {
            lv_scr_load(_screen);
            lv_obj_invalidate(_screen); // Force redraw
            ESP_LOGI(TAG, "Screen loaded");
        } else {
            ESP_LOGE(TAG, "Screen is NULL on Open");
        }
    }

    void onRunning() override {
        _update_button_state();

        if (HAL::Millis() - _last_update_time > 200) {
            HAL::UpdatePowerMonitor();
            auto data = HAL::GetPowerMonitorData();
            
            float t1 = HAL::GetNTC(0);
            float t2 = HAL::GetNTC(1);

            // Update UI
            if (_label_power) {
                char buf[128];
                snprintf(buf, sizeof(buf), "V: %.2f V\nI: %.3f A\nP: %.3f W", 
                    data.busVoltage, data.shuntCurrent, data.busPower);
                lv_label_set_text(_label_power, buf);
            }

            if (_label_ntc) {
                char buf[64];
                snprintf(buf, sizeof(buf), "T1: %.1f C\nT2: %.1f C", t1, t2);
                lv_label_set_text(_label_ntc, buf);
            }

            if (_label_fan) {
                char buf[32];
                snprintf(buf, sizeof(buf), "Fan: %s %d%%", _fan_enabled ? "ON" : "OFF", _fan_speed);
                lv_label_set_text(_label_fan, buf);
            }

            if (_label_led) {
                const char* modes[] = {"OFF", "RED", "GREEN", "BLUE", "WHITE", "RAND"};
                char buf[32];
                snprintf(buf, sizeof(buf), "LED: %s", modes[_led_mode]);
                lv_label_set_text(_label_led, buf);
                
                // Keep changing color in random mode
                if (_led_mode == 5) {
                    // Update color target randomly every 200ms
                    HAL::SetLed(esp_random()%256, esp_random()%256, esp_random()%256, esp_random()%256);
                }
            }
            
            // Log output
            // ESP_LOGI(TAG, "Power: V=%.2f V, I=%.3f A, P=%.3f W | T1=%.1f T2=%.1f", 
            //          data.busVoltage, data.shuntCurrent, data.busPower, t1, t2);

            _last_update_time = HAL::Millis();
        }
    }

    void onDestroy() override {
        ESP_LOGI(TAG, "onDestroy");
        if (_screen) lv_obj_del(_screen);
        _screen = nullptr;
    }
};
