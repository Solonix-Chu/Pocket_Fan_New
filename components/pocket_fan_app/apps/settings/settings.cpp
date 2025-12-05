#include "settings.h"
#include "app_button.h"

void SettingsApp::onCreate()
{
    setAppInfo().name = "Settings";
}

void SettingsApp::_create_ui()
{
    if (_screen) return;
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_black(), 0);
    lv_obj_set_style_text_color(_screen, lv_color_white(), 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* menu_cont = lv_obj_create(_screen);
    lv_obj_set_size(menu_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(menu_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(menu_cont, 0, 0);
    lv_obj_clear_flag(menu_cont, LV_OBJ_FLAG_SCROLLABLE);

    _selector_obj = lv_obj_create(menu_cont);
    lv_obj_set_style_bg_color(_selector_obj, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_30, 0);
    lv_obj_set_style_radius(_selector_obj, 5, 0);
    lv_obj_set_style_border_width(_selector_obj, 1, 0);
    lv_obj_set_style_border_color(_selector_obj, lv_color_white(), 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    _data.option_list.clear();
    _option_labels.clear();

    int y = 10;
    for (const auto& item : _settings_items) {
        Option_t option;
        option.keyframe = {10, (float)y, 108, 30};
        addOption(option);

        lv_obj_t* label = lv_label_create(menu_cont);
        lv_label_set_text(label, item.c_str());
        lv_obj_align(label, LV_ALIGN_TOP_LEFT, 20, y + 8);
        _option_labels.push_back(label);

        y += 35;
    }

    setConfig().cameraSize = {128, 64};
    setConfig().renderInterval = 10;
    setConfig().readInputInterval = 20;
    setConfig().moveInLoop = false;

    jumpTo(0);
}

void SettingsApp::onOpen()
{
    _create_ui();
    lv_scr_load(_screen);
}

void SettingsApp::onRunning()
{
    update();
}

void SettingsApp::onClose()
{
    _destroy_ui();
}

void SettingsApp::onDestroy()
{
    _destroy_ui();
}

void SettingsApp::_destroy_ui()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        _selector_obj = nullptr;
        _option_labels.clear();
    }
}

void SettingsApp::onReadInput()
{
    if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        goNext();
    } else if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        goLast();
    } else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        auto current = getSelectorCurrentFrame();
        press({current.x + 5, current.y + 5, current.width - 10, current.height - 10});
    } else if (BtnOk->currentState == APP_BUTTON_STATE_NOCHANGE && isPressing()) {
        release();
    }
}

void SettingsApp::onRender()
{
    if (!_screen) return;
    
    auto selector_frame = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, selector_frame.x, selector_frame.y);
    lv_obj_set_size(_selector_obj, selector_frame.width, selector_frame.height);
    
    auto camera = getCameraOffset();
    lv_obj_t* menu_cont = lv_obj_get_parent(_selector_obj);
    lv_obj_set_pos(menu_cont, -camera.x, -camera.y);
}

void SettingsApp::onClick()
{
    int selected = getSelectedOptionIndex();
    if (selected == _settings_items.size() - 1) { // Back
        AppAbility::close();
    } else {
        // Toggle things
        printf("Clicked setting: %s\n", _settings_items[selected].c_str());
    }
}
