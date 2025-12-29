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
    
    // Add items (Based on VAMeter root.cpp)
    _view->addSettingsItem({"About", [this]() {
        ESP_LOGI(TAG, "Open About");
    }});
    
    _view->addSettingsItem({"Display", [this]() {
        ESP_LOGI(TAG, "Open Display Settings");
    }});

    _view->addSettingsItem({"Buzzer", [this]() {
        ESP_LOGI(TAG, "Open Buzzer Settings");
    }});

    _view->addSettingsItem({"Encoder", [this]() {
        ESP_LOGI(TAG, "Open Encoder Settings");
    }});

    _view->addSettingsItem({"Language", [this]() {
        ESP_LOGI(TAG, "Toggle Language");
    }});

    _view->addSettingsItem({"Back", [this]() {
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
