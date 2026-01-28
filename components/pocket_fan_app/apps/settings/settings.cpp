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
        auto btn_right = HAL::GetButton(BUTTON::BTN_RIGHT);
        auto btn_left = HAL::GetButton(BUTTON::BTN_LEFT);
        // Intercept inputs for brightness
        if (btn_right == APP_BUTTON_STATE_CLICKED || btn_left == APP_BUTTON_STATE_CLICKED) {
            uint32_t now = HAL::Millis();
            uint32_t delta = (_last_scroll_time == 0) ? 1000 : (now - _last_scroll_time);
            _last_scroll_time = now;

            int step = 10;
            if (delta < 30) step = 30;
            else if (delta < 50) step = 20;
            else if (delta < 100) step = 15;
            else if (delta < 200) step = 10;
            else step = 5;

            if (btn_right == APP_BUTTON_STATE_CLICKED) {
                if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
                _brightness_val += step;
            } else {
                if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
                _brightness_val -= step;
            }

            if (_brightness_val > 255) _brightness_val = 255;
            if (_brightness_val < 0) _brightness_val = 0;
            HAL::SetDisplayBrightness(static_cast<uint8_t>(_brightness_val));
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
    _invert_display = HAL::GetSystemConfig().invertDisplay;
    _is_black_theme = HAL::GetSystemConfig().darkTheme;
    int item_index = 0;

    // 0. Operation Guide (New)
    _view->addSettingsItem({"Operation Guide", false, false, []() {
        ESP_LOGI(TAG, "Open Guide");
        AppStartupAnim::PopUpGuideMap(true);
    }});
    item_index++;

    // 1. Long text auto-scroll example
    _view->addSettingsItem({"System Settings and Configuration Menu", false, false, []() {
        ESP_LOGI(TAG, "Clicked long text item");
    }});
    item_index++;

    // 2. Brightness
    _view->addSettingsItem({"Brightness", false, false, [this]() {
        ESP_LOGI(TAG, "Open Brightness Adjust");
        _is_adjusting_brightness = true;
        _last_scroll_time = 0;
        _view->showBrightnessPopup((_brightness_val * 100) / 255);
    }});
    item_index++;
    
    // 3. Theme Toggle with Checkbox
    _theme_item_index = item_index;
    _view->addSettingsItem({"Dark Theme", true, _is_black_theme, [this]() {
        _is_black_theme = !_is_black_theme;
        ESP_LOGI(TAG, "Toggle Theme: %s", _is_black_theme ? "Black" : "White");
        _view->updateItemValue(_theme_item_index, _is_black_theme);
        HAL::GetSystemConfig().darkTheme = _is_black_theme;
        HAL::SaveSystemConfig();
    }});
    item_index++;

    // 4. Monochrome invert toggle (SSD1306)
    _invert_item_index = item_index;
    _view->addSettingsItem({"Invert Display", true, _invert_display, [this]() {
        _invert_display = !_invert_display;
        ESP_LOGI(TAG, "Toggle Invert Display: %s", _invert_display ? "ON" : "OFF");
        HAL::SetDisplayInvert(_invert_display);
        HAL::GetSystemConfig().invertDisplay = _invert_display;
        HAL::SaveSystemConfig();
        _view->updateItemValue(_invert_item_index, _invert_display);
    }});
    item_index++;

    // 4. Language Toggle
    _view->addSettingsItem({"Language: EN", false, false, [this]() {
        static bool is_en = true;
        is_en = !is_en;
        ESP_LOGI(TAG, "Switch Language: %s", is_en ? "EN" : "CN");
        LocaleCode_t locale = is_en ? locale_code_en : locale_code_cn;
        AssetPool::SetLocaleCode(locale);
        HAL::GetSystemConfig().localeCode = locale;
        HAL::SaveSystemConfig();
        // TODO: Update current UI text if possible, or wait for next app open
    }});
    item_index++;

    // 5. Back
    _view->addSettingsItem({"Back", false, false, [this]() {
        ESP_LOGI(TAG, "Back to Menu");
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    item_index++;
    
    _view->init();
}

void SettingsApp::_destroy_view()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}
