#include "settings.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
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
    
    // 1. Long text auto-scroll example
    _view->addSettingsItem({"A very long settings item text that will scroll circularly", false, false, []() {
        ESP_LOGI(TAG, "Clicked long text item");
    }});

    // 2. Brightness
    _view->addSettingsItem({"Brightness", false, false, [this]() {
        ESP_LOGI(TAG, "Open Brightness Adjust");
        // TODO: popup modal
    }});
    
    // 3. Theme Toggle with Checkbox
    bool is_black_theme = false; // TODO: Get from HAL config
    _view->addSettingsItem({"Dark Theme", true, is_black_theme, [this, is_black_theme]() {
        static bool black = is_black_theme;
        black = !black;
        ESP_LOGI(TAG, "Toggle Theme: %s", black ? "Black" : "White");
        _view->updateItemValue(2, black);
        // TODO: Apply global theme
    }});

    // 4. Language Toggle
    _view->addSettingsItem({"Language: EN", false, false, [this]() {
        static bool is_en = true;
        is_en = !is_en;
        ESP_LOGI(TAG, "Switch Language: %s", is_en ? "EN" : "CN");
        // TODO: Apply language switch
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
