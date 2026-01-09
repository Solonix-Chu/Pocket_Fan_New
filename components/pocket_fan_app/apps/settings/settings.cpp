#include "settings.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include "../app_startup_anim/app_startup_anim.h"
#include <esp_log.h>

static const char* TAG = "SettingsApp";

void SettingsApp::onCreate()
{
    setAppInfo().name = "Settings";
}

void SettingsApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _create_view();
}

void SettingsApp::onRunning()
{
    if (_is_adjusting_brightness) {
        // Intercept inputs for brightness
        if (HAL::GetButton(BUTTON::BTN_UP) == APP_BUTTON_STATE_CLICKED) {
            if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
            _brightness_val += 10;
            if (_brightness_val > 255) _brightness_val = 255;
            HAL::GetDisplay()->setBrightness(_brightness_val);
            _view->updateBrightnessPopup((_brightness_val * 100) / 255);
        }
        else if (HAL::GetButton(BUTTON::BTN_DOWN) == APP_BUTTON_STATE_CLICKED) {
            if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
            _brightness_val -= 10;
            if (_brightness_val < 0) _brightness_val = 0;
            HAL::GetDisplay()->setBrightness(_brightness_val);
            _view->updateBrightnessPopup((_brightness_val * 100) / 255);
        }
        else if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
            if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
            ESP_LOGI(TAG, "Brightness applied: %d", _brightness_val);
            HAL::GetSystemConfig().brightness = _brightness_val;
            HAL::SaveSystemConfig();
            _view->hideBrightnessPopup();
            _is_adjusting_brightness = false;
        }
        if (_view) {
            _view->update();
        }
        return;
    }

    if (_view) {
        _view->update();
    }
}

void SettingsApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    _destroy_view();
}

void SettingsApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
}

void SettingsApp::_create_view()
{
    if (_view) return;
    _view = new SettingsView();
    
    // Get initial values
    _brightness_val = HAL::GetSystemConfig().brightness;

    // 0. Operation Guide (New)
    _view->addSettingsItem({"Operation Guide", false, false, []() {
        ESP_LOGI(TAG, "Open Guide");
        AppStartupAnim::PopUpGuideMap(true);
    }});

    // 1. Long text auto-scroll example
    _view->addSettingsItem({"System Settings and Configuration Menu", false, false, []() {
        ESP_LOGI(TAG, "Clicked long text item");
    }});

    // 2. Brightness
    _view->addSettingsItem({"Brightness", false, false, [this]() {
        ESP_LOGI(TAG, "Open Brightness Adjust");
        _is_adjusting_brightness = true;
        _view->showBrightnessPopup((_brightness_val * 100) / 255);
    }});
    
    // 3. Theme Toggle with Checkbox
    static bool is_black_theme = false; 
    _view->addSettingsItem({"Dark Theme", true, is_black_theme, [this]() {
        is_black_theme = !is_black_theme;
        ESP_LOGI(TAG, "Toggle Theme: %s", is_black_theme ? "Black" : "White");
        _view->updateItemValue(2, is_black_theme);
        // HAL doesn't have theme yet, just toggle checkbox for now
    }});

    // 4. Language Toggle
    _view->addSettingsItem({"Language: EN", false, false, [this]() {
        static bool is_en = true;
        is_en = !is_en;
        ESP_LOGI(TAG, "Switch Language: %s", is_en ? "EN" : "CN");
        AssetPool::SetLocaleCode(is_en ? locale_code_en : locale_code_cn);
        HAL::SaveSystemConfig();
        // TODO: Update current UI text if possible, or wait for next app open
    }});

    // 5. Back
    _view->addSettingsItem({"Back", false, false, [this]() {
        ESP_LOGI(TAG, "Back to Menu");
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    
    _view->init();
}

void SettingsApp::_destroy_view()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}
