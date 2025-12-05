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

class HardwareTestApp : public mooncake::AppAbility {
private:
    static constexpr const char* TAG = "HardwareTest";
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _label_buttons = nullptr;
    lv_obj_t* _label_title = nullptr;

    std::string _last_status = "Press buttons...";

    void _update_button_state() {
        if (!_label_buttons) return;

        bool changed = false;
        std::string current_status = "Buttons:\n";
        
        struct BtnInfo {
            BUTTON::Button_t id; 
            const char* name;
        };
        
        BtnInfo btns[] = {
            {BUTTON::BTN_UP, "UP"},
            {BUTTON::BTN_DOWN, "DOWN"},
            {BUTTON::BTN_LEFT, "LEFT"},
            {BUTTON::BTN_RIGHT, "RIGHT"},
            {BUTTON::BTN_MID, "OK/MID"}
        };

        for (auto& btn : btns) {
            app_button_state_t state = HAL::GetButton((BUTTON::Button_t)btn.id);
            
            if (state != APP_BUTTON_STATE_NOCHANGE) {
                current_status += std::string(btn.name) + ": ";
                switch(state) {
                    case APP_BUTTON_STATE_CLICKED: current_status += "CLICK"; break;
                    case APP_BUTTON_STATE_HOLD: current_status += "HOLD"; break;
                    case APP_BUTTON_STATE_DECIDE_CLICK_COUNT: current_status += "DBL_WAIT"; break;
                    default: current_status += std::to_string((int)state); break;
                }
                current_status += "\n";
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

        // _label_title = lv_label_create(_screen);
        // lv_label_set_text(_label_title, "Hardware Test");
        // lv_obj_set_style_text_color(_label_title, lv_color_black(), 0);
        // lv_obj_align(_label_title, LV_ALIGN_TOP_MID, 0, 10);

        _label_buttons = lv_label_create(_screen);
        lv_label_set_text(_label_buttons, "Press buttons...");
        lv_obj_set_style_text_color(_label_buttons, lv_color_black(), 0);
        lv_obj_align(_label_buttons, LV_ALIGN_CENTER, 0, 10);
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
        // Update button states
        // HAL::Get()->allButton_refresh(); // Not needed if called in app_update? 
        // Actually app.cpp calls app_button_update(), so we don't need to call it here unless we want faster poll inside app?
        // app_button_update is global. 
        // Let's rely on system loop calling app_button_update.
        // But wait, `HardwareTestApp` runs in `app_update` loop via `GetMooncake().update()`.
        // `app_update` in `app.cpp` calls `GetMooncake().update()` then `app_button_update()`.
        // So button update happens AFTER app update.
        // This means app sees OLD state (from previous frame). That's fine.
        
        _update_button_state();
    }

    void onDestroy() override {
        ESP_LOGI(TAG, "onDestroy");
        if (_screen) lv_obj_del(_screen);
        _screen = nullptr;
    }
};
