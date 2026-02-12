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
    _entry_y.setDurationMs(320);
    _entry_y.setEasing(ease::ease_out_back);
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
    // Re-enable entry animation from below
    restartEntry();
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
    lv_obj_set_style_translate_y(_tileview, 80, 0);

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

    // Page 3
    _tile3 = lv_tileview_add_tile(_tileview, 2, 0, LV_DIR_HOR);
    _bg3 = lv_obj_create(_tile3);
    lv_obj_set_size(_bg3, 128, 64);
    lv_obj_set_style_bg_color(_bg3, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_bg3, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_bg3, 0, 0);
    lv_obj_clear_flag(_bg3, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(_bg3, LV_ALIGN_CENTER, 0, 0);

    // Data Labels on Page 1
    // Ref positions from setup_scr_screen_1.c
    // screen_1_label_1 (V): pos(-9, 15), size(67, 15)
    // screen_1_label_2 (A): pos(-9, 31), size(67, 18)
    // screen_1_label_3 (W): pos(-9, 47), size(67, 18)
    
    _label_v = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_v, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_v, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_v, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_v, 67, 15);
    lv_obj_set_pos(_label_v, -9, 15);
    lv_label_set_text(_label_v, "0.00");

    _label_a = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_a, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_a, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_a, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_a, 67, 18);
    lv_obj_set_pos(_label_a, -9, 30);
    lv_label_set_text(_label_a, "0.00");

    _label_w = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_w, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_w, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_w, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_w, 67, 18);
    lv_obj_set_pos(_label_w, -9, 45);
    lv_label_set_text(_label_w, "0.00");

    // PWM Label on Page 1 (Right side)
    _label_pwm = lv_label_create(_tile1);
    lv_obj_set_style_text_color(_label_pwm, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_pwm, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_pwm, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_pwm, 40, 18);
    lv_obj_set_pos(_label_pwm, 80, 25);
    lv_label_set_text(_label_pwm, "0%");

    // Page 2 labels (T, Cap)
    _label_t = lv_label_create(_tile2);
    lv_obj_set_style_text_color(_label_t, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_t, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_t, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_t, 67, 18);
    lv_obj_set_pos(_label_t, 3, 45);
    lv_label_set_text(_label_t, "0.0");

    _label_cap = lv_label_create(_tile2);
    lv_obj_set_style_text_color(_label_cap, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_cap, AssetPool::GetLocaleFontLarge(), 0);
    lv_obj_set_style_text_align(_label_cap, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_size(_label_cap, 64, 18);
    lv_obj_set_pos(_label_cap, 58, 23);
    lv_label_set_text(_label_cap, "0.00");

    // Page 3 labels (IP2369 telemetry, demo style)
    _label_ip_title = lv_label_create(_tile3);
    lv_obj_set_style_text_color(_label_ip_title, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_ip_title, AssetPool::GetLocaleFontSmall(), 0);
    lv_obj_set_style_text_align(_label_ip_title, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(_label_ip_title, 124, 12);
    lv_obj_set_pos(_label_ip_title, 0, 0);
    lv_label_set_text(_label_ip_title, "IP2369 IDLE");

    _label_ip_in = lv_label_create(_tile3);
    lv_obj_set_style_text_color(_label_ip_in, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_ip_in, AssetPool::GetLocaleFontSmall(), 0);
    lv_obj_set_style_text_align(_label_ip_in, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(_label_ip_in, 124, 12);
    lv_obj_set_pos(_label_ip_in, 0, 12);
    lv_label_set_text(_label_ip_in, "IN : --.-");

    _label_ip_out = lv_label_create(_tile3);
    lv_obj_set_style_text_color(_label_ip_out, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_ip_out, AssetPool::GetLocaleFontSmall(), 0);
    lv_obj_set_style_text_align(_label_ip_out, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(_label_ip_out, 124, 12);
    lv_obj_set_pos(_label_ip_out, 0, 24);
    lv_label_set_text(_label_ip_out, "OUT: --.-");

    _label_ip_power = lv_label_create(_tile3);
    lv_obj_set_style_text_color(_label_ip_power, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_ip_power, AssetPool::GetLocaleFontSmall(), 0);
    lv_obj_set_style_text_align(_label_ip_power, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(_label_ip_power, 124, 12);
    lv_obj_set_pos(_label_ip_power, 0, 36);
    lv_label_set_text(_label_ip_power, "P: --.-/--.-W");

    _label_ip_ntc = lv_label_create(_tile3);
    lv_obj_set_style_text_color(_label_ip_ntc, lv_color_black(), 0);
    lv_obj_set_style_text_font(_label_ip_ntc, AssetPool::GetLocaleFontSmall(), 0);
    lv_obj_set_style_text_align(_label_ip_ntc, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_size(_label_ip_ntc, 124, 12);
    lv_obj_set_pos(_label_ip_ntc, 0, 48);
    lv_label_set_text(_label_ip_ntc, "NTC: --.-C");

    // Force labels to foreground
    lv_obj_move_foreground(_label_v);
    lv_obj_move_foreground(_label_a);
    lv_obj_move_foreground(_label_w);
    lv_obj_move_foreground(_label_pwm);
    lv_obj_move_foreground(_label_t);
    lv_obj_move_foreground(_label_cap);
    lv_obj_move_foreground(_label_ip_title);
    lv_obj_move_foreground(_label_ip_in);
    lv_obj_move_foreground(_label_ip_out);
    lv_obj_move_foreground(_label_ip_power);
    lv_obj_move_foreground(_label_ip_ntc);
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

void HomepageView::updateIp2369(const std::string& title,
                                const std::string& in,
                                const std::string& out,
                                const std::string& power,
                                const std::string& ntc) {
    if (_label_ip_title) lv_label_set_text(_label_ip_title, title.c_str());
    if (_label_ip_in) lv_label_set_text(_label_ip_in, in.c_str());
    if (_label_ip_out) lv_label_set_text(_label_ip_out, out.c_str());
    if (_label_ip_power) lv_label_set_text(_label_ip_power, power.c_str());
    if (_label_ip_ntc) lv_label_set_text(_label_ip_ntc, ntc.c_str());
}

void HomepageView::updatePwm(int dutyCycle) {
    if (_label_pwm) {
        lv_label_set_text_fmt(_label_pwm, "%d%%", dutyCycle);
    }
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

void HomepageView::tick(uint32_t now_ms) {
    if (_tileview && _entry_started) {
        _entry_y.updateMs(now_ms);
        lv_obj_set_style_translate_y(_tileview, (lv_coord_t)_entry_y.value(), 0);
        if (_entry_y.isFinished()) {
            _entry_started = false;
            lv_obj_set_style_translate_y(_tileview, 0, 0);
        }
    }
}

void HomepageView::restartEntry() {
    _entry_started = true;
    _entry_y.jumpTo(80);
    _entry_y.moveTo(0);
}

void HomepageView::onReadInput() {
    // Input handling is delegated to the App or handled here if specific to view navigation.
    // For Homepage, App will handle transitions to Menu.
}

void HomepageView::onClick() {
}
