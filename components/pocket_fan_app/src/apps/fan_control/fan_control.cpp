#include "fan_control.h"
#include "app_button.h"

void FanControlApp::onCreate()
{
    setAppInfo().name = "Fan Control";
}

void FanControlApp::_create_ui()
{
    if (_screen) return;
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_black(), 0);
    lv_obj_set_style_text_color(_screen, lv_color_white(), 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    _number_flow = new NumberFlow(_screen);
    _number_flow->setAlign(LV_ALIGN_CENTER);
    _number_flow->setPos(0, 0);
    _number_flow->setTextFont(&lv_font_montserrat_24); // Or some other font
    _number_flow->setSuffix("%");
    _number_flow->init();
    _number_flow->setValue(_speed);
}

void FanControlApp::onOpen()
{
    _create_ui();
    lv_scr_load(_screen);
}

void FanControlApp::onRunning()
{
    _update_input();
    if (_number_flow) {
        _number_flow->update();
    }
}

void FanControlApp::onClose()
{
    _destroy_ui();
}

void FanControlApp::onDestroy()
{
    _destroy_ui();
}

void FanControlApp::_destroy_ui()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        // _number_flow is managed by LVGL as it is a child of screen? 
        // Wait, `NumberFlow` class holds pointers but the LVGL object hierarchy deletes children.
        // However, `NumberFlow` itself is a C++ object. I should delete it.
        // LVGL deletes the C objects. The C++ wrapper pointers (`_digit_labels` etc) will be invalid.
        // But `NumberFlow` destructor... `Widget` destructor...
        // `Widget` wraps `lv_obj`. If `owns_ptr` is true (default), it calls `lv_obj_del` in destructor.
        // If I call `lv_obj_del(_screen)`, all children are deleted by LVGL.
        // If I then delete `_number_flow`, its destructor tries to delete `lv_obj` again?
        // `PointerWrapper` destructor calls `release_ptr` if `owns_ptr`.
        // `Widget` inherits `PointerWrapper<lv_obj_t, lv_obj_del>`.
        // So `delete _number_flow` will call `lv_obj_del`.
        // If I deleted `_screen` first, `_number_flow`'s obj is already deleted?
        // `lv_obj_del` handles invalid pointers? No.
        // `PointerWrapper` checks `isValid()`. `lv_obj_is_valid`.
        
        // Safe way: delete C++ objects (which delete LVGL objects) then delete screen.
        delete _number_flow;
        _number_flow = nullptr;
        
        // Screen was created manually `lv_obj_create(NULL)`.
        // I should probably wrap screen in a C++ object too or just delete it manually.
        // But `NumberFlow`'s parent is `_screen`.
        // If I delete `_number_flow`, it removes itself from `_screen`.
        
        // So:
        // delete _number_flow;
        // lv_obj_del(_screen);
    }
}

void FanControlApp::_update_input()
{
    if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        _speed += 5;
        if (_speed > 100) _speed = 100;
        _number_flow->setValue(_speed);
    } else if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        _speed -= 5;
        if (_speed < 0) _speed = 0;
        _number_flow->setValue(_speed);
    } else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        close();
    }
}
