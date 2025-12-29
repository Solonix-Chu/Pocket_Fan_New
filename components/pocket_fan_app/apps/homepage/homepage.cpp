#include "homepage.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
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
    // Transition to Menu on OK button press
    if (HAL::GetButton(BUTTON::BTN_OK) == APP_BUTTON_STATE_PRESS) {
        ESP_LOGI(TAG, "OK button pressed, opening MenuApp");
        // TODO: App switching logic
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
    // TODO: Create LVGL view
}

void HomepageApp::_destroy_view()
{
    // TODO: Destroy LVGL view
}
