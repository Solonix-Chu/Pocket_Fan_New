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
    if (HAL::GetButton(BUTTON::BTN_OK) == APP_BUTTON_STATE_CLICKED) {
        ESP_LOGI(TAG, "OK button pressed, opening MenuApp");
        GetMooncake().openApp(1);
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
