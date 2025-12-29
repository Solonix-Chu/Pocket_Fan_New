#include "menu.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include <esp_log.h>

static const char* TAG = "MenuApp";

void MenuApp::onCreate()
{
    setAppInfo().name = "Menu";
}

void MenuApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _last_input_time = HAL::GetTick();
    _create_view();
}

void MenuApp::onRunning()
{
    // Update last input time if any button is pressed or wheel is turned
    if (HAL::GetButton(BUTTON::BTN_OK) != APP_BUTTON_STATE_NOCHANGE ||
        HAL::GetButton(BUTTON::BTN_LEFT) != APP_BUTTON_STATE_NOCHANGE ||
        HAL::GetButton(BUTTON::BTN_RIGHT) != APP_BUTTON_STATE_NOCHANGE) {
        _last_input_time = HAL::GetTick();
    }

    // Timeout: 5s
    if (HAL::GetTick() - _last_input_time > 5000) {
        ESP_LOGI(TAG, "Inactivity timeout, returning to Homepage");
        GetMooncake().openApp(0);
        return;
    }

    // Long press OK to return to homepage
    if (HAL::GetButton(BUTTON::BTN_OK) == APP_BUTTON_STATE_HOLD) {
        ESP_LOGI(TAG, "OK button hold detected, returning to Homepage");
        GetMooncake().openApp(0);
        return;
    }

    if (_view) {
        _view->update();
    }
}

void MenuApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    _destroy_view();
}

void MenuApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
}

void MenuApp::_create_view()
{
    if (_view) return;

    _view = new MenuView();
    
    // Add options
    _view->addSettingsOption({AssetPool::GetImgSetting(), "General"});
    _view->addSettingsOption({AssetPool::GetImgDetail(), "Detail"});
    _view->addSettingsOption({AssetPool::GetImgHealth(), "Health"});
    _view->addSettingsOption({AssetPool::GetImgEmoji(), "Emoji"});
    _view->addSettingsOption({AssetPool::GetImgEnjoy(), "Enjoy"});
    _view->addSettingsOption({AssetPool::GetImgAbout(), "About"});
    _view->addSettingsOption({AssetPool::GetImgQuit(), "Quit"});

    // Callback
    _view->setOpenCallback([this](int index) {
        ESP_LOGI(TAG, "Selected index: %d", index);
        if (index == 6) { // Quit
            close(); 
        } else {
            // TODO: Open sub-menus
        }
    });

    _view->init();
}

void MenuApp::_destroy_view()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}