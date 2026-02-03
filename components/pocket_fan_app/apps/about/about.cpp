#include "about.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include "app_button.h"
#include <esp_log.h>
#include <esp_mac.h>
#include <cstdio>

static const char* TAG = "AboutApp";
static constexpr const char* kBleOtaName = "nimble-ble-ota";

void AboutApp::onCreate()
{
    setAppInfo().name = "About";
}

void AboutApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");

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

    _screen = lv_obj_create(NULL);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);

    const lv_font_t* title_font = AssetPool::GetGullyBold12();
    const lv_font_t* body_font = AssetPool::GetGullyBold12();

    _label_title = lv_label_create(_screen);
    // Always keep this screen ASCII-only (no Chinese strings)
    lv_label_set_text(_label_title, "OTA Upgrade");
    lv_obj_set_pos(_label_title, 4, 0);
    lv_obj_set_style_text_font(_label_title, title_font, 0);

    _label_fw = lv_label_create(_screen);
    lv_obj_set_pos(_label_fw, 4, 10);
    lv_obj_set_style_text_font(_label_fw, body_font, 0);
    lv_obj_set_width(_label_fw, 120);
    lv_label_set_long_mode(_label_fw, LV_LABEL_LONG_SCROLL_CIRCULAR);

    _label_name = lv_label_create(_screen);
    lv_label_set_text_fmt(_label_name, "Name: %s", kBleOtaName);
    lv_obj_set_pos(_label_name, 4, 20);
    lv_obj_set_style_text_font(_label_name, body_font, 0);

    _label_mac = lv_label_create(_screen);
    lv_label_set_text_fmt(_label_mac, "%s", _mac_str);
    lv_obj_set_pos(_label_mac, 4, 30);
    lv_obj_set_style_text_font(_label_mac, body_font, 0);

    _label_status = lv_label_create(_screen);
    lv_obj_set_pos(_label_status, 4, 40);
    lv_obj_set_style_text_font(_label_status, body_font, 0);
    lv_obj_set_width(_label_status, 120);
    lv_label_set_long_mode(_label_status, LV_LABEL_LONG_SCROLL_CIRCULAR);

    _label_progress = lv_label_create(_screen);
    lv_obj_set_pos(_label_progress, 4, 50);
    lv_obj_set_style_text_font(_label_progress, body_font, 0);
    lv_obj_set_width(_label_progress, 120);
    lv_label_set_long_mode(_label_progress, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_set_style_text_color(_label_title, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_fw, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_name, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_mac, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_status, lv_color_black(), 0);
    lv_obj_set_style_text_color(_label_progress, lv_color_black(), 0);

    lv_scr_load(_screen);
    _update_labels();
}

void AboutApp::_update_labels()
{
    if (!_screen) return;

    lv_label_set_text_fmt(_label_fw, "FW: %s  Build: %s", HAL::Version().c_str(), HAL::CompileDate().c_str());

    const auto status = HAL::GetBleOtaStatus();
    if (status.state == BLE_OTA::STATE_WRITING) {
        lv_label_set_text(_label_status, "Status: Writing...");
        if (status.total_len > 0) {
            const uint32_t pct = (status.received_len * 100) / status.total_len;
            lv_label_set_text_fmt(_label_progress, "Progress: %lu/%lu (%lu%%)",
                                  (unsigned long)status.received_len,
                                  (unsigned long)status.total_len,
                                  (unsigned long)pct);
        } else {
            lv_label_set_text_fmt(_label_progress, "Progress: %lu bytes",
                                  (unsigned long)status.received_len);
        }
        return;
    }

    if (status.state == BLE_OTA::STATE_READY) {
        lv_label_set_text(_label_status, "Status: Waiting connect...");
        lv_label_set_text(_label_progress, "MID: Exit");
        return;
    }

    if (status.init_error != 0) {
        lv_label_set_text_fmt(_label_status, "Status: BLE init failed (%ld)", (long)status.init_error);
        lv_label_set_text(_label_progress, "Check BT/PSRAM config");
        return;
    }

    lv_label_set_text(_label_status, "Status: Starting...");
    lv_label_set_text(_label_progress, "");
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
}
