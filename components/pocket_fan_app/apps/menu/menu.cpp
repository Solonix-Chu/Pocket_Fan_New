#include "menu.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include <esp_log.h>

static const char* TAG = "MenuApp";
static int s_menu_last_index = 0;
static bool s_menu_skip_entry = false;
static int s_menu_deferred_open_app = -1;

void MenuApp::onCreate()
{
    setAppInfo().name = "Menu";
}

void MenuApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _last_input_time = HAL::Millis();
    _exit_to_app = false;
    _exit_to_home = false;
    _create_view();
}

void MenuApp::onRunning()
{
    // Update last input time if any button is pressed or wheel is turned
    if (HAL::GetButton(BUTTON::BTN_MID) != APP_BUTTON_STATE_NOCHANGE ||
        HAL::GetButton(BUTTON::BTN_LEFT) != APP_BUTTON_STATE_NOCHANGE ||
        HAL::GetButton(BUTTON::BTN_RIGHT) != APP_BUTTON_STATE_NOCHANGE) {
        _last_input_time = HAL::Millis();
    }

    // Timeout: 5s
    if (HAL::Millis() - _last_input_time > 5000) {
        ESP_LOGI(TAG, "Inactivity timeout, triggering exit anim");
        _view->startExitAnimation([this]() {
            ESP_LOGI(TAG, "Exit anim done, returning to Homepage");
            _exit_to_home = true;
            _exit_to_app = false;
            mooncake::GetMooncake().openApp(APPS::homepage_id);
            close();
        });
        // Reset time to prevent multiple triggers while animating
        _last_input_time = HAL::Millis();
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

    if (s_menu_deferred_open_app >= 0) {
        mooncake::GetMooncake().openApp(s_menu_deferred_open_app);
        s_menu_deferred_open_app = -1;
    }
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

    const auto& tr = AssetPool::GetText();
    
    // Add options
    _view->addSettingsOption({AssetPool::GetImgSetting(), tr.PocketFan_Menu_General});
    _view->addSettingsOption({AssetPool::GetImgDetail(), tr.PocketFan_Menu_Detail});
    _view->addSettingsOption({AssetPool::GetImgHealth(), tr.PocketFan_Menu_Health});
    _view->addSettingsOption({AssetPool::GetImgEmoji(), tr.PocketFan_Menu_Emoji});
    _view->addSettingsOption({AssetPool::GetImgEnjoy(), tr.PocketFan_Menu_Enjoy});
    _view->addSettingsOption({AssetPool::GetImgAbout(), tr.PocketFan_Menu_About});
    _view->addSettingsOption({AssetPool::GetImgQuit(), tr.PocketFan_Menu_Quit});

    // Callback
    _view->setOpenCallback([this](int index) {
        ESP_LOGI(TAG, "Selected index: %d", index);
        if (index == 0) { // Settings/General
            _exit_to_app = true;
            _exit_to_home = false;
            mooncake::GetMooncake().openApp(APPS::settings_id);
            close();
        } else if (index == 6) { // Quit
            _exit_to_home = true;
            _exit_to_app = false;
            mooncake::GetMooncake().openApp(APPS::homepage_id);
            close(); 
        } else if (index == 4) { // Enjoy
            _exit_to_app = true;
            _exit_to_home = false;
            mooncake::GetMooncake().openApp(APPS::enjoy_id);
            close();
        } else if (index == 2) { // Health
            _exit_to_app = true;
            _exit_to_home = false;
            mooncake::GetMooncake().openApp(APPS::health_id);
            close();
        } else if (index == 5) { // About (BLE OTA)
            _exit_to_app = true;
            _exit_to_home = false;
            // Defer open to next frame so MenuView resources are freed first.
            s_menu_deferred_open_app = APPS::about_id;
            close();
        } else {
            // TODO: Open sub-menus
        }
    });

    _view->setInitialIndex(s_menu_last_index);
    _view->setSkipEntryAnimation(s_menu_skip_entry);
    _view->init();
}

void MenuApp::_destroy_view()
{
    if (_view) {
        if (_exit_to_app) {
            s_menu_last_index = _view->getSelectedOptionIndex();
            s_menu_skip_entry = true;
        } else {
            s_menu_last_index = 0;
            s_menu_skip_entry = false;
        }
        delete _view;
        _view = nullptr;
    }
}
