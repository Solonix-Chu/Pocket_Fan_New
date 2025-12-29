#include "homepage_view.h"
#include "app_button.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"
#include <esp_log.h>

static const char* TAG = "HomepageView";

HomepageView::HomepageView() {
    // Config
    setConfig().cameraSize = {128, 64};
    setConfig().renderInterval = 15;
    setConfig().readInputInterval = 0; // Immediate input
}

HomepageView::~HomepageView() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void HomepageView::init() {
    _create_lvgl_objects();
    
    // Load screen
    lv_scr_load(_screen);
}

void HomepageView::_create_lvgl_objects() {
    if (_screen) return;

    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    _tileview = lv_tileview_create(_screen);
    lv_obj_set_size(_tileview, 128, 64);
    lv_obj_set_align(_tileview, LV_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(_tileview, LV_OPA_TRANSP, 0);
    lv_obj_remove_flag(_tileview, LV_OBJ_FLAG_SCROLLABLE);

    // Page 1
    _tile1 = lv_tileview_add_tile(_tileview, 0, 0, LV_DIR_HOR);
    _bg1 = lv_image_create(_tile1);
    lv_image_set_src(_bg1, AssetPool::GetImgHomePage1());
    lv_obj_align(_bg1, LV_ALIGN_CENTER, 0, 0);

    // Page 2
    _tile2 = lv_tileview_add_tile(_tileview, 1, 0, LV_DIR_HOR);
    _bg2 = lv_image_create(_tile2);
    lv_image_set_src(_bg2, AssetPool::GetImgHomePage2());
    lv_obj_align(_bg2, LV_ALIGN_CENTER, 0, 0);

    // Data Labels on Page 1
    // Ref positions from setup_scr_screen_1.c
    // screen_1_label_1 (V): pos(-9, 15), size(67, 15)
    // screen_1_label_2 (A): pos(-9, 31), size(67, 18)
    // screen_1_label_3 (W): pos(-9, 47), size(67, 18)
    
    _label_v = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_v, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_v, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_align(_label_v, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_v, 67, 15);
    lv_obj_set_pos(_label_v, -9, 15);
    lv_label_set_text(_label_v, "0.00");

    _label_a = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_a, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_a, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_align(_label_a, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_a, 67, 18);
    lv_obj_set_pos(_label_a, -9, 31);
    lv_label_set_text(_label_a, "0.00");

    _label_w = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_w, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_w, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_align(_label_w, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_w, 67, 18);
    lv_obj_set_pos(_label_w, -9, 47);
    lv_label_set_text(_label_w, "0.00");

    // Page 2 labels (T, Cap)
    // screen_1_label_4 (T): pos(3, 109 - 64? = 45), size(67, 18) - Wait, screen_1 is 128x128 in demo
    // Homepage is 128x64. So Page 2 is 0-63.
    // label_4 pos(3, 109) -> relative to page 2 (y=64) is 45.
    // label_5 pos(58, 87) -> relative to page 2 is 23.
    
    _label_t = lv_label_create(_tile2);
    lv_obj_set_style_text_color(_label_t, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_t, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_align(_label_t, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_t, 67, 18);
    lv_obj_set_pos(_label_t, 3, 45);
    lv_label_set_text(_label_t, "0.0");

    _label_cap = lv_label_create(_tile2);
    lv_obj_set_style_text_color(_label_cap, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_cap, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_align(_label_cap, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_cap, 64, 18);
    lv_obj_set_pos(_label_cap, 58, 23);
    lv_label_set_text(_label_cap, "0.00");

    // Force labels to foreground
    lv_obj_move_foreground(_label_v);
    lv_obj_move_foreground(_label_a);
    lv_obj_move_foreground(_label_w);
}

void HomepageView::updateData(const std::string& v, const std::string& a, const std::string& w) {
    if (_label_v) lv_label_set_text(_label_v, v.c_str());
    if (_label_a) lv_label_set_text(_label_a, a.c_str());
    if (_label_w) lv_label_set_text(_label_w, w.c_str());
}

void HomepageView::updatePage2Data(const std::string& t, const std::string& cap) {
    if (_label_t) lv_label_set_text(_label_t, t.c_str());
    if (_label_cap) lv_label_set_text(_label_cap, cap.c_str());
}

void HomepageView::setPage(int page) {
    if (_tileview) {
        lv_obj_set_tile_id(_tileview, page, 0, LV_ANIM_ON);
    }
}

void HomepageView::onRender() {
    // SmoothSelector logic not strictly needed for this simple dashboard,
    // but we can use it if we want smooth moving elements.
}

void HomepageView::onReadInput() {
    // Input handling is delegated to the App or handled here if specific to view navigation.
    // For Homepage, App will handle transitions to Menu.
}

void HomepageView::onClick() {
}
