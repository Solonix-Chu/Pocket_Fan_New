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

    // Data Labels on Screen (Overlay) or Tile1?
    // If we want them only on Page 1, they should be children of Tile1 or bg1.
    // To ensure they are visible ON TOP of bg1, we create them after bg1.
    
    _label_v = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_v, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_v, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_v, 10, 5);
    lv_label_set_text(_label_v, "--V");

    _label_a = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_a, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_a, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_a, 10, 25);
    lv_label_set_text(_label_a, "--A");

    _label_w = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_w, lv_color_white(), 0);
    lv_obj_set_style_text_font(_label_w, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_pos(_label_w, 10, 45);
    lv_label_set_text(_label_w, "--W");

    // Force labels to foreground just in case
    lv_obj_move_foreground(_label_v);
    lv_obj_move_foreground(_label_a);
    lv_obj_move_foreground(_label_w);
}

void HomepageView::updateData(const std::string& v, const std::string& a, const std::string& w) {
    if (_label_v) lv_label_set_text(_label_v, v.c_str());
    if (_label_a) lv_label_set_text(_label_a, a.c_str());
    if (_label_w) lv_label_set_text(_label_w, w.c_str());
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
