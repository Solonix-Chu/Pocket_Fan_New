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
    // Update telemetry labels every 200ms
    if (HAL::Millis() - _last_update_time > 200) {
        _last_update_time = HAL::Millis();

        auto pm_data = HAL::GetPowerMonitorData();
        
        auto v = HAL::GetUnitAdaptatedVoltage(pm_data.busVoltage);
        lv_label_set_text_fmt(_label_v, "%s%s", v.value.c_str(), v.unit.c_str());

        auto a = HAL::GetUnitAdaptatedCurrent(pm_data.shuntCurrent);
        lv_label_set_text_fmt(_label_a, "%s%s", a.value.c_str(), a.unit.c_str());

        auto w = HAL::GetUnitAdaptatedPower(pm_data.busPower);
        lv_label_set_text_fmt(_label_w, "%s%s", w.value.c_str(), w.unit.c_str());
    }

    // Switch between pages using wheel (left/right buttons as wheel surrogates or raw wheel events)
    if (HAL::GetButton(BUTTON::BTN_RIGHT) == APP_BUTTON_STATE_CLICKED) {
        if (_current_page == 0) {
            _current_page = 1;
            lv_obj_set_tile_id(_tileview, 1, 0, LV_ANIM_ON);
        }
    } else if (HAL::GetButton(BUTTON::BTN_LEFT) == APP_BUTTON_STATE_CLICKED) {
        if (_current_page == 1) {
            _current_page = 0;
            lv_obj_set_tile_id(_tileview, 0, 0, LV_ANIM_ON);
        }
    }

    // Transition to Menu on OK button press
    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        ESP_LOGI(TAG, "OK button pressed, opening MenuApp");
        mooncake::GetMooncake().openApp(2);
    }
}

void HomepageApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    // Don't destroy view here if we want to preserve state? 
    // Usually, onClose should hide or destroy if needed.
    _destroy_view();
}

void HomepageApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
}

void HomepageApp::_create_view()
{
    if (_screen) return;

    _screen = lv_obj_create(NULL);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    _tileview = lv_tileview_create(_screen);
    lv_obj_set_size(_tileview, 128, 64);
    lv_obj_set_align(_tileview, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(_tileview, LV_OPA_TRANSP, 0);
    // Disable user drag to force wheel control
    lv_obj_remove_flag(_tileview, LV_OBJ_FLAG_SCROLLABLE);

    // Page 1
    _tile1 = lv_tileview_add_tile(_tileview, 0, 0, LV_DIR_HOR);
    lv_obj_t* bg1 = lv_image_create(_tile1);
    lv_image_set_src(bg1, AssetPool::GetImgHomePage1());
    lv_obj_align(bg1, LV_ALIGN_CENTER, 0, 0);

    // Data Labels on Page 1 (Child of bg1 to ensure on top/relative positioning)
    _label_v = lv_label_create(bg1);
    lv_obj_set_style_text_color(_label_v, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_v, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_v, 10, 5);
    lv_label_set_text(_label_v, "0.00V");

    _label_a = lv_label_create(bg1);
    lv_obj_set_style_text_color(_label_a, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_a, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_a, 10, 25);
    lv_label_set_text(_label_a, "0.00A");

    _label_w = lv_label_create(bg1);
    lv_obj_set_style_text_color(_label_w, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_w, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_w, 10, 45);
    lv_label_set_text(_label_w, "0.00W");

    // Page 2
    _tile2 = lv_tileview_add_tile(_tileview, 1, 0, LV_DIR_HOR);
    lv_obj_t* bg2 = lv_image_create(_tile2);
    lv_image_set_src(bg2, AssetPool::GetImgHomePage2());
    lv_obj_align(bg2, LV_ALIGN_CENTER, 0, 0);

    lv_scr_load(_screen);
}

void HomepageApp::_destroy_view()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}
