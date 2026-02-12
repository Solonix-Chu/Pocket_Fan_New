#include "about.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include "app_button.h"
#include <esp_log.h>
#include <esp_mac.h>
#include <cstdio>

static const char* TAG = "AboutApp";

void AboutApp::onCreate()
{
    setAppInfo().name = "About";
}

void AboutApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");

    // Reset render cache each time the view is rebuilt so the initial label texts
    // are always applied (and marquee scroll can run smoothly afterwards).
    _last_state = 0xFF;
    _last_received_len = static_cast<uint32_t>(-1);
    _last_total_len = static_cast<uint32_t>(-1);
    _last_init_error = static_cast<int32_t>(0x7fffffff);
    _last_progress_update_ms = 0;

    uint8_t mac[6] = {0};
    esp_read_mac(mac, ESP_MAC_BT);
    snprintf(_mac_str, sizeof(_mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    HAL::StartBleOta();
    _create_view();
}

void AboutApp::onRunning()
{
    _update_labels();

    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;

        if (_can_exit()) {
            ESP_LOGI(TAG, "Exit BLE OTA screen -> Menu");
            mooncake::GetMooncake().openApp(APPS::menu_id);
            close();
            return;
        }

        ESP_LOGI(TAG, "OTA in progress, ignore exit click");
    }
}

void AboutApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    _destroy_view();
    HAL::StopBleOta();
}

void AboutApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
    HAL::StopBleOta();
}

bool AboutApp::_can_exit() const
{
    const auto status = HAL::GetBleOtaStatus();
    return status.state != BLE_OTA::STATE_WRITING;
}

void AboutApp::_create_view()
{
    if (_screen) return;

    const auto& tr = AssetPool::GetText();

    _screen = lv_obj_create(NULL);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);

    // Use a locale-capable font (CN needs CJK font).
    const lv_font_t* font = AssetPool::GetLocaleFontSmall();

    _label_title = lv_label_create(_screen);
    lv_label_set_text(_label_title, tr.AppSettings_Option_OTA);
    lv_obj_set_pos(_label_title, 4, 0);
    lv_obj_set_style_text_font(_label_title, font, 0);

    _label_fw = lv_label_create(_screen);
    lv_obj_set_pos(_label_fw, 4, 15);
    lv_obj_set_style_text_font(_label_fw, font, 0);
    lv_obj_set_width(_label_fw, 120);
    lv_label_set_long_mode(_label_fw, LV_LABEL_LONG_SCROLL_CIRCULAR);

    _label_status = lv_label_create(_screen);
    lv_obj_set_pos(_label_status, 4, 30);
    lv_obj_set_style_text_font(_label_status, font, 0);
    lv_obj_set_width(_label_status, 120);
    lv_label_set_long_mode(_label_status, LV_LABEL_LONG_SCROLL_CIRCULAR);

    _label_progress = lv_label_create(_screen);
    lv_obj_set_pos(_label_progress, 4, 45);
    lv_obj_set_style_text_font(_label_progress, font, 0);
    lv_obj_set_width(_label_progress, 120);
    lv_label_set_long_mode(_label_progress, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_set_style_text_color(_label_title, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_fw, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_status, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_progress, lv_color_black(), 0);

    // FW/version string is static for the lifetime of the screen: setting it once
    // avoids restarting LVGL's LV_LABEL_LONG_SCROLL_CIRCULAR animation every frame.
    lv_label_set_text_fmt(_label_fw, tr.PocketFan_About_InfoFmt,
                          HAL::Version().c_str(), HAL::CompileDate().c_str(), _mac_str);

    lv_scr_load(_screen);
    _update_labels();
}

void AboutApp::_update_labels()
{
    if (!_screen) return;

    const auto& tr = AssetPool::GetText();

    const auto status = HAL::GetBleOtaStatus();

    const bool state_changed =
        (status.state != static_cast<BLE_OTA::State>(_last_state)) || (status.init_error != _last_init_error);

    if (status.state == BLE_OTA::STATE_WRITING) {
        if (state_changed) {
            lv_label_set_text(_label_status, tr.PocketFan_About_Status_Writing);
        }

        // Progress can update frequently during OTA. Throttle to keep UI smooth and
        // avoid constantly resetting any marquee scrolling on the label.
        const bool progress_changed =
            (status.received_len != _last_received_len) || (status.total_len != _last_total_len);
        const unsigned long now = HAL::Millis();
        const bool allow_progress_update = state_changed || (now - _last_progress_update_ms) >= 120;

        if (progress_changed && allow_progress_update) {
            if (status.total_len > 0) {
                const uint32_t pct = (status.received_len * 100) / status.total_len;
                lv_label_set_text_fmt(_label_progress, tr.PocketFan_About_ProgressFmt,
                                      (unsigned long)status.received_len,
                                      (unsigned long)status.total_len,
                                      (unsigned long)pct);
            } else {
                lv_label_set_text_fmt(_label_progress, tr.PocketFan_About_ProgressBytesFmt,
                                      (unsigned long)status.received_len);
            }
            _last_received_len = status.received_len;
            _last_total_len = status.total_len;
            _last_progress_update_ms = now;
        }

        _last_state = status.state;
        _last_init_error = status.init_error;
        return;
    }

    if (status.state == BLE_OTA::STATE_READY) {
        if (state_changed) {
            lv_label_set_text(_label_status, tr.PocketFan_About_Status_Waiting);
            lv_label_set_text(_label_progress, tr.PocketFan_About_MidExit);
        }
        _last_state = status.state;
        _last_init_error = status.init_error;
        return;
    }

    if (status.init_error != 0) {
        if (state_changed) {
            lv_label_set_text_fmt(_label_status, tr.PocketFan_About_Status_BleInitFailedFmt, (long)status.init_error);
            lv_label_set_text(_label_progress, tr.PocketFan_About_CheckBtPsram);
        }
        _last_state = status.state;
        _last_init_error = status.init_error;
        return;
    }

    // STATE_IDLE without init_error.
    if (state_changed) {
        lv_label_set_text(_label_status, tr.PocketFan_About_Status_Starting);
        lv_label_set_text(_label_progress, "");
    }
    _last_state = status.state;
    _last_init_error = status.init_error;
}

void AboutApp::_destroy_view()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
    _label_title = nullptr;
    _label_fw = nullptr;
    _label_name = nullptr;
    _label_mac = nullptr;
    _label_status = nullptr;
    _label_progress = nullptr;

    // Reset cache so next open forces a full label refresh.
    _last_state = 0xFF;
    _last_received_len = static_cast<uint32_t>(-1);
    _last_total_len = static_cast<uint32_t>(-1);
    _last_init_error = static_cast<int32_t>(0x7fffffff);
    _last_progress_update_ms = 0;
}
