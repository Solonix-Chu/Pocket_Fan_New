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
        
        // Staggered Entrance Anim
        for (size_t i = 0; i < _item_transitions.size(); i++) {
            _item_transitions[i].teleport(-128, getOptionList()[i].keyframe.y);
            _item_transitions[i].move(getOptionList()[i].keyframe.x, getOptionList()[i].keyframe.y);
            _item_transitions[i].x.delay = 0.1f + i * 0.03f;
            _item_transitions[i].x.easingOptions().duration = 0.4f;
            _item_transitions[i].x.easingOptions().easingFunction = ease::ease_out_back;
        }

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
    // Start from top-left (0,0)
    option.keyframe.x = 0; 
    option.keyframe.y = index * (_item_h + _item_gap); 
    option.keyframe.width = 128; // Default, will be updated dynamically
    option.keyframe.height = _item_h;
    addOption(option);

    // Staggered transition setup
    _item_transitions.emplace_back();
}

void SettingsView::updateItemValue(int index, bool checked) {
    if (index >= 0 && index < (int)_checkbox_objs.size() && _checkbox_objs[index]) {
        if (checked) {
            lv_obj_add_state(_checkbox_objs[index], LV_STATE_CHECKED);
        } else {
            lv_obj_remove_state(_checkbox_objs[index], LV_STATE_CHECKED);
        }
    }
}

void SettingsView::_create_lvgl_objects() {
    if (_screen) return;

    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    _list_cont = lv_obj_create(_screen);
    // Allow container to grow with items
    lv_obj_set_size(_list_cont, 128, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_list_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_list_cont, 0, 0);
    lv_obj_clear_flag(_list_cont, LV_OBJ_FLAG_SCROLLABLE);

    _selector_obj = lv_obj_create(_list_cont);
    // Solid black background for color inversion effect
    lv_obj_set_style_bg_color(_selector_obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(_selector_obj, 4, 0);
    lv_obj_set_style_border_width(_selector_obj, 0, 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    for (size_t i = 0; i < _items_props.size(); i++) {
        const auto& kf = getOptionList()[i].keyframe;

        // Label
        lv_obj_t* label = lv_label_create(_list_cont);
        lv_label_set_text(label, _items_props[i].name.c_str());
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_font(label, AssetPool::GetGullyBold16(), 0);
        
        // Measure width for selective marquee
        lv_obj_update_layout(label);
        int text_w = lv_obj_get_self_width(label);
        
        // Update keyframe width for selector (text width + padding)
        _data.option_list[i].keyframe.width = text_w + 12;

        if (text_w > 110) {
            lv_obj_set_width(label, 110);
            lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        } else {
            lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
        }
        
        lv_obj_set_pos(label, (int32_t)kf.x + 4, (int32_t)kf.y + 4);
        _item_objs.push_back(label);

        // Checkbox
        if (_items_props[i].has_checkbox) {
            lv_obj_t* cb = lv_checkbox_create(_list_cont);
            lv_checkbox_set_text(cb, "");
            lv_obj_set_pos(cb, 100, (int32_t)kf.y + 2);
            if (_items_props[i].checked) {
                lv_obj_add_state(cb, LV_STATE_CHECKED);
            }
            _checkbox_objs.push_back(cb);
        } else {
            _checkbox_objs.push_back(nullptr);
        }
    }
}

void SettingsView::onRender() {
    if (!_screen) return;
    int32_t trans_x = (int32_t)_transition_offset.value();
    auto camera = getCameraOffset();

    // Update Selector
    auto selector = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, 
        (int32_t)(selector.x - _selector_pad), 
        (int32_t)(selector.y - _selector_pad));
    lv_obj_set_size(_selector_obj, 
        (int32_t)(selector.width + _selector_pad * 2), 
        (int32_t)(selector.height + _selector_pad * 2));

    // Update Camera (move container) + Transition Offset
    lv_obj_set_pos(_list_cont, -(int32_t)camera.x + trans_x, -(int32_t)camera.y);

    // Color Inversion and Staggered Position Update
    int selected_idx = getSelectedOptionIndex();
    for (size_t i = 0; i < _item_objs.size(); i++) {
        auto pos = _item_transitions[i].value();
        lv_obj_set_pos(_item_objs[i], (int32_t)pos.x + 4, (int32_t)pos.y + 4);
        if (_checkbox_objs[i]) {
            lv_obj_set_pos(_checkbox_objs[i], (int32_t)pos.x + 100, (int32_t)pos.y + 2);
        }

        if (i == (size_t)selected_idx) {
            lv_obj_set_style_text_color(_item_objs[i], lv_color_white(), 0);
        } else {
            lv_obj_set_style_text_color(_item_objs[i], lv_color_black(), 0);
        }
    }

    // Popup transition
    if (_is_popup_active && _popup_cont) {
        lv_obj_set_style_translate_y(_popup_cont, (int32_t)_popup_transition.value(), 0);
    }
}

void SettingsView::onUpdate(const uint32_t& currentTime) {
    float current_time_s = currentTime / 1000.0f;
    _transition_offset.update(current_time_s);
    _popup_transition.update(current_time_s);
    for (auto& item : _item_transitions) {
        item.update(current_time_s);
    }
}

void SettingsView::showBrightnessPopup(int initialValue) {
    if (_is_popup_active) return;
    _is_popup_active = true;

    // Create popup container
    _popup_cont = lv_obj_create(_screen);
    lv_obj_set_size(_popup_cont, 100, 40);
    lv_obj_align(_popup_cont, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(_popup_cont, lv_color_white(), 0);
    lv_obj_set_style_border_color(_popup_cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(_popup_cont, 2, 0);
    lv_obj_set_style_radius(_popup_cont, 8, 0);
    lv_obj_clear_flag(_popup_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Progress bar
    _popup_bar = lv_bar_create(_popup_cont);
    lv_obj_set_size(_popup_bar, 80, 10);
    lv_obj_align(_popup_bar, LV_ALIGN_CENTER, 0, 5);
    lv_bar_set_range(_popup_bar, 0, 100);
    lv_bar_set_value(_popup_bar, initialValue, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(_popup_bar, lv_color_black(), LV_PART_INDICATOR);

    // Label
    _popup_label = lv_label_create(_popup_cont);
    lv_obj_set_style_text_font(_popup_label, AssetPool::GetGullyBold16(), 0);
    lv_obj_set_style_text_color(_popup_label, lv_color_black(), 0);
    lv_obj_align(_popup_label, LV_ALIGN_TOP_MID, 0, 0);
    lv_label_set_text_fmt(_popup_label, "BRI: %d%%", initialValue);

    // Entrance Anim
    _popup_transition.teleport(64); // Start from below
    _popup_transition.move(0);
    _popup_transition.easingOptions().duration = 0.3f;
    _popup_transition.easingOptions().easingFunction = ease::ease_out_back;
}

void SettingsView::updateBrightnessPopup(int value) {
    if (!_is_popup_active) return;
    if (_popup_bar) lv_bar_set_value(_popup_bar, value, LV_ANIM_OFF);
    if (_popup_label) lv_label_set_text_fmt(_popup_label, "BRI: %d%%", value);
}

void SettingsView::hideBrightnessPopup() {
    if (!_is_popup_active) return;
    if (_popup_cont) {
        lv_obj_del(_popup_cont);
        _popup_cont = nullptr;
        _popup_bar = nullptr;
        _popup_label = nullptr;
    }
    _is_popup_active = false;
}

void SettingsView::onReadInput() {
    if (isOpening()) return;

    // If popup is active, we don't allow menu navigation
    if (_is_popup_active) {
        // App will handle the actual adjustment logic by calling HAL and then updateBrightnessPopup
        return;
    }

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
    // Keep selection at the very top
    auto targetY = getSelectedKeyframe().y;
    getCamera().move(0, targetY);
}
