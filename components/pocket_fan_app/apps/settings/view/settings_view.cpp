#include "settings_view.h"
#include "app_button.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"
#include <esp_log.h>

static const char* TAG = "SettingsView";

SettingsView::SettingsView() {
    setConfig().cameraSize = {128, 64};
    setConfig().renderInterval = 15;
    setConfig().readInputInterval = 0;
    setConfig().moveInLoop = true;
}

SettingsView::~SettingsView() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void SettingsView::init() {
    _create_lvgl_objects();
    lv_scr_load(_screen);
    
    if (!_items_props.empty()) {
        jumpTo(0);
        _transition_offset.teleport(-128);
        _transition_offset.move(0);
        _transition_offset.easingOptions().duration = 0.4f;
        onRender();
    }
}

void SettingsView::addSettingsItem(const SettingsItemProps& props) {
    _items_props.push_back(props);
    
    Option_t option;
    int index = _items_props.size() - 1;
    // Each item is 24px high + 4px gap. Start at y=10.
    option.keyframe.x = 5; 
    option.keyframe.y = index * (_item_h + _item_gap) + 10; 
    option.keyframe.width = 118;
    option.keyframe.height = _item_h;
    addOption(option);
}

void SettingsView::_create_lvgl_objects() {
    if (_screen) return;

    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    _list_cont = lv_obj_create(_screen);
    lv_obj_set_size(_list_cont, 128, 64);
    lv_obj_set_style_bg_opa(_list_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_list_cont, 0, 0);
    lv_obj_clear_flag(_list_cont, LV_OBJ_FLAG_SCROLLABLE);

    _selector_obj = lv_obj_create(_list_cont);
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(_selector_obj, 4, 0);
    lv_obj_set_style_border_color(_selector_obj, lv_color_black(), 0);
    lv_obj_set_style_border_width(_selector_obj, 1, 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < _items_props.size(); i++) {
        lv_obj_t* label = lv_label_create(_list_cont);
        lv_label_set_text(label, _items_props[i].name.c_str());
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_font(label, AssetPool::GetGullyBold16(), 0);
        
        const auto& kf = getOptionList()[i].keyframe;
        // Text padding inside item
        lv_obj_set_pos(label, (int32_t)kf.x + 4, (int32_t)kf.y + 4);
        _item_objs.push_back(label);
    }
}

void SettingsView::onRender() {
    if (!_screen) return;
    int32_t trans_x = (int32_t)_transition_offset.value();
    auto camera = getCameraOffset();

    // Update Selector (Adjusted for camera offset if needed, 
    // but usually selector is relative to container)
    auto selector = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, 
        (int32_t)(selector.x - _selector_pad), 
        (int32_t)(selector.y - _selector_pad));
    lv_obj_set_size(_selector_obj, 
        (int32_t)(selector.width + _selector_pad * 2), 
        (int32_t)(selector.height + _selector_pad * 2));

    // Update Camera (move container) + Transition Offset
    lv_obj_set_pos(_list_cont, -(int32_t)camera.x + trans_x, -(int32_t)camera.y);
}

void SettingsView::onUpdate(const uint32_t& currentTime) {
    _transition_offset.update(currentTime / 1000.0f);
}

void SettingsView::onReadInput() {
    if (isOpening()) return;
    if (HAL::GetButton(BUTTON::BTN_DOWN) == APP_BUTTON_STATE_CLICKED) {
        if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        goNext();
    } else if (HAL::GetButton(BUTTON::BTN_UP) == APP_BUTTON_STATE_CLICKED) {
        if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        goLast();
    } else if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        onClick();
    }
}

void SettingsView::onClick() {
    ESP_LOGI(TAG, "onClick");
    // Expand animation
    open({0 + getCameraOffset().x, 0, 128, 64});
    
    float duration = 0.15f;
    getSelectorPostion().x.easingOptions().duration = duration;
    getSelectorPostion().y.easingOptions().duration = duration;
    getSelectorShape().x.easingOptions().duration = duration;
    getSelectorShape().y.easingOptions().duration = duration;
}

void SettingsView::onOpenEnd() {
    ESP_LOGI(TAG, "onOpenEnd");
    int idx = getSelectedOptionIndex();
    if (idx >= 0 && idx < _items_props.size()) {
        if (_items_props[idx].callback) _items_props[idx].callback();
    }
    
    // Reset state so it can be clicked again
    close();
}

void SettingsView::_update_camera_keyframe() {
    auto targetY = getSelectedKeyframe().y - (64 - _item_h)/2;
    getCamera().move(0, targetY);
}
