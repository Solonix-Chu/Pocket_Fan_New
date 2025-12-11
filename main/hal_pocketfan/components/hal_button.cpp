/*
* SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
*
* SPDX-License-Identifier: MIT
*/
#include "../hal_pocketfan.h"
#include <mooncake.h>
#include "app_button.h"
#include "esp_log.h"

static const char *TAG = "hal_button";

void HAL_PocketFan::_button_init()
{
    ESP_LOGI(TAG,"button init");
    app_button_init();
}

app_button_state_t HAL_PocketFan::getButton(BUTTON::Button_t button)
{
    // Map HAL button enum to app_button instances
    app_button_t* target_btn = nullptr;

    switch (button)
    {
    case BUTTON::BTN_UP:
        target_btn = BtnUp;
        break;
    case BUTTON::BTN_DOWN:
        target_btn = BtnDown;
        break;
    case BUTTON::BTN_LEFT:
        target_btn = BtnLeft;
        break;
    case BUTTON::BTN_RIGHT:
        target_btn = BtnRight;
        break;
    case BUTTON::BTN_MID:
        target_btn = BtnOk; // Assuming MID maps to OK/Select
        break;
    case BUTTON::BTN_POWER:
        target_btn = BtnPower;
        break;
    default:
        return APP_BUTTON_STATE_NOCHANGE;
    }

    if (target_btn) {
        // Return the synchronized state from the last update()
        return target_btn->currentState;
    }
    return APP_BUTTON_STATE_NOCHANGE;
}

void HAL_PocketFan::allButton_refresh()
{
    app_button_update();
}
