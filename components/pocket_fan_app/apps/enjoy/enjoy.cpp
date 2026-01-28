#include "enjoy.h"
#include "../../hal/hal.h"
#include "../apps.h"
#include <esp_random.h>

EnjoyApp::EnjoyApp() {
    setAppInfo().name = "Enjoy";
    setAppInfo().icon = nullptr;
}

void EnjoyApp::onOpen() {
    _screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_display_t* disp = lv_display_get_default();
    lv_color_t bg_color = lv_color_white();
    lv_color_t ball_color = lv_color_black();
    lv_obj_set_style_bg_color(_screen, bg_color, 0);

    _ball = lv_obj_create(_screen);
    lv_obj_set_size(_ball, (int)(_radius * 2), (int)(_radius * 2));
    lv_obj_set_style_radius(_ball, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(_ball, ball_color, 0);
    lv_obj_set_style_bg_opa(_ball, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(_ball, 8, 0);
    lv_obj_set_style_shadow_color(_ball, lv_color_make(0x30, 0x30, 0x30), 0);
    lv_obj_set_style_border_width(_ball, 0, 0);
    lv_obj_clear_flag(_ball, LV_OBJ_FLAG_SCROLLABLE);

    // _hint = lv_label_create(_screen);
    // // lv_label_set_text(_hint, "Roll to bounce\nOK to exit");
    // lv_obj_set_style_text_color(_hint, lv_color_black(), 0);
    // lv_obj_align(_hint, LV_ALIGN_TOP_MID, 0, 2);

    if (disp) {
        _screen_w = (float)lv_display_get_horizontal_resolution(disp);
        _screen_h = (float)lv_display_get_vertical_resolution(disp);
    } else {
        _screen_w = 128.0f;
        _screen_h = 64.0f;
    }

    if (_screen_w < _radius * 2.0f) _screen_w = _radius * 2.0f;
    if (_screen_h < _radius * 2.0f) _screen_h = _radius * 2.0f;

    _x = _screen_w * 0.5f;
    _y = _screen_h * 0.5f;
    _vx = 35.0f;
    _vy = -20.0f;
    _last_ms = HAL::Millis();
    _updateBall();
    lv_scr_load(_screen);
    lv_obj_invalidate(_screen);
}

void EnjoyApp::_applyInput(float dt) {
    const float impulse = 70.0f;

    auto applyImpulse = [&](float ix, float iy) {
        _vx += ix;
        _vy += iy;
    };

    auto state_left = HAL::GetButton(BUTTON::BTN_LEFT);
    auto state_right = HAL::GetButton(BUTTON::BTN_RIGHT);
    auto state_up = HAL::GetButton(BUTTON::BTN_UP);
    auto state_down = HAL::GetButton(BUTTON::BTN_DOWN);

    if (state_left == APP_BUTTON_STATE_CLICKED) {
        applyImpulse(-impulse, 0);
        if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
    }
    if (state_right == APP_BUTTON_STATE_CLICKED) {
        applyImpulse(impulse, 0);
        if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
    }
    if (state_up == APP_BUTTON_STATE_CLICKED) {
        applyImpulse(0, -impulse);
        if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
    }
    if (state_down == APP_BUTTON_STATE_CLICKED) {
        applyImpulse(0, impulse);
        if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
    }
}

void EnjoyApp::_bounceColor() {
    // uint32_t r = esp_random() % 256;
    // uint32_t g = esp_random() % 256;
    // uint32_t b = esp_random() % 256;
    uint32_t r = 256;
    uint32_t g = 256;
    uint32_t b = 256;
    lv_obj_set_style_bg_color(_ball, lv_color_make(r, g, b), 0);
}

void EnjoyApp::_stepPhysics(float dt) {
    const float friction = 0.9f;
    _vx *= (1.0f - friction * dt);
    _vy *= (1.0f - friction * dt);

    _x += _vx * dt;
    _y += _vy * dt;

    float min_x = _radius;
    float max_x = _screen_w - _radius;
    float min_y = _radius;
    float max_y = _screen_h - _radius;

    bool bounced = false;

    if (_x < min_x) {
        _x = min_x;
        _vx = -_vx * 0.8f;
        bounced = true;
    } else if (_x > max_x) {
        _x = max_x;
        _vx = -_vx * 0.8f;
        bounced = true;
    }

    if (_y < min_y) {
        _y = min_y;
        _vy = -_vy * 0.8f;
        bounced = true;
    } else if (_y > max_y) {
        _y = max_y;
        _vy = -_vy * 0.8f;
        bounced = true;
    }

    if (bounced) {
        _bounceColor();
        // Add a tiny random spin to keep it lively
        _vx += (int32_t)(esp_random() % 11 - 5);
        _vy += (int32_t)(esp_random() % 11 - 5);
    }
}

void EnjoyApp::_updateBall() {
    lv_obj_set_pos(_ball, (int)(_x - _radius), (int)(_y - _radius));
}

void EnjoyApp::onRunning() {
    // Exit on OK
    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
        return;
    }

    uint32_t now = HAL::Millis();
    float dt = (_last_ms == 0) ? 0.0f : (now - _last_ms) / 1000.0f;
    _last_ms = now;
    if (dt > 0.05f) dt = 0.05f; // clamp to avoid big jumps

    _applyInput(dt);
    _stepPhysics(dt);
    _updateBall();
}

void EnjoyApp::onClose() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        _ball = nullptr;
        _hint = nullptr;
    }
}

void EnjoyApp::onDestroy() {
    onClose();
}
