#include "menu_view.h"
#include "app_button.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"
#include <esp_log.h>
#include <utility>

MenuView::MenuView() {
    // Config
    setConfig().cameraSize = {128, 64}; // 128x64 Screen
    setConfig().renderInterval = 15;
    setConfig().readInputInterval = 0; // Read input every frame to catch transient button states
    setConfig().moveInLoop = true;
    
    // Setup transitions (Easing)
    getSelectorPostion().x.easingOptions().duration = 0.3f;
    getSelectorPostion().x.easingOptions().easingFunction = ease::ease_out_quad;
    getSelectorPostion().y.easingOptions().duration = 0.3f;
    getSelectorPostion().y.easingOptions().easingFunction = ease::ease_out_quad;

    getSelectorShape().x.easingOptions().duration = 0.3f;
    getSelectorShape().y.easingOptions().duration = 0.3f;

    getCamera().x.easingOptions().duration = 0.4f;
    getCamera().x.easingOptions().easingFunction = ease::ease_out_cubic;
    getCamera().y.easingOptions().duration = 0.4f;

    _entry_offset.setDurationMs(400);
    _entry_offset.setEasing(ease::ease_out_back);

    _label_slide.setDurationMs(320);
    _label_slide.setEasing(ease::ease_out_back);
}

MenuView::~MenuView() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void MenuView::init() {
    _create_lvgl_objects();
    
    // Load screen
    lv_scr_load(_screen);
    
    // Initial jump
    if (!_settings_props.empty()) {
        jumpTo(0);

        _icon_transitions.resize(_settings_props.size());
        for (size_t i = 0; i < _settings_props.size(); i++) {
            const auto& kf = getOptionList()[i].keyframe;
            auto& trans = _icon_transitions[i];
            trans.setDurationMs(380);
            trans.setDelayMs(80 + static_cast<uint32_t>(i) * 40);
            trans.setEasing(ease::ease_out_back);
            trans.jumpTo(kf.x - 24, kf.y + 12);
            trans.moveTo(kf.x, kf.y);
        }

        // Entrance Anim: Slide from left using transition offset
        _entry_offset.setDelayMs(0);
        _entry_offset.jumpTo(-128);
        _entry_offset.moveTo(0);

        _label_slide.setDelayMs(150);
        _label_slide.jumpTo(18);
        _label_slide.moveTo(0);
        
        onRender(); // Render immediately to prevent flash at 0
    }
}

void MenuView::startExitAnimation(std::function<void()> callback) {
    ESP_LOGI("MenuView", "startExitAnimation");
    // Slide out to left
    _entry_offset.setDurationMs(280);
    _entry_offset.setDelayMs(0);
    _entry_offset.setCompleteCallback(std::move(callback));
    _entry_offset.moveTo(-128);

    _label_slide.setDurationMs(240);
    _label_slide.setDelayMs(0);
    _label_slide.setEasing(ease::ease_out_cubic);
    _label_slide.moveTo(-64);

    for (size_t i = 0; i < _icon_transitions.size(); i++) {
        auto base = getOptionList()[i].keyframe;
        _icon_transitions[i].setDurationMs(260);
        _icon_transitions[i].setDelayMs(0);
        _icon_transitions[i].setEasing(ease::ease_out_cubic);
        _icon_transitions[i].moveTo(base.x - 24, base.y + 10);
    }
}

void MenuView::addSettingsOption(const SettingsOptionProps& props) {
    _settings_props.push_back(props);

    Option_t option;
    // Layout: Horizontal row
    int index = _settings_props.size() - 1;
    option.keyframe.x = index * (_icon_w + _icon_gap) + 128 / 2 - _icon_w / 2; 
    option.keyframe.y = (64 / 2 - _icon_h / 2) - 15; 

    option.keyframe.width = _icon_w;
    option.keyframe.height = _icon_h;
    
    addOption(option);
}

void MenuView::_create_lvgl_objects() {
    if (_screen) return;

    _screen = lv_obj_create(NULL);
    // White background
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0); 
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    // Menu Container (will be moved by camera)
    _menu_cont = lv_obj_create(_screen);
    lv_obj_set_size(_menu_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_menu_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_menu_cont, 0, 0);
    lv_obj_clear_flag(_menu_cont, LV_OBJ_FLAG_SCROLLABLE);

    // Selector
    _selector_obj = lv_obj_create(_menu_cont);
    // Transparent background, only border
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(_selector_obj, 8, 0);
    lv_obj_set_style_border_color(_selector_obj, lv_color_black(), 0); // Black border
    lv_obj_set_style_border_width(_selector_obj, 2, 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    // Icons
    for (size_t i = 0; i < _settings_props.size(); i++) {
        lv_obj_t* img = lv_image_create(_menu_cont);
        if (_settings_props[i].icon) {
            lv_image_set_src(img, _settings_props[i].icon);
        }
        
        // Position icons based on their keyframes
        const auto& kf = getOptionList()[i].keyframe;
        lv_obj_set_pos(img, (int32_t)kf.x, (int32_t)kf.y);
        _icon_objs.push_back(img);
    }
    
    // Label for current option name
    _label_obj = lv_label_create(_screen);
    // Black text for white background
    lv_obj_set_style_text_color(_label_obj, lv_color_black(), 0);
    // Use the custom font from AssetPool
    lv_obj_set_style_text_font(_label_obj, AssetPool::GetGullyBold16(), 0);
    
    lv_obj_align(_label_obj, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_label_set_text(_label_obj, "");
}

void MenuView::onRender() {
    if (!_screen) return;
    
    int32_t trans_x = (int32_t)_entry_offset.value();

    // Update Selector
    auto selector = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, 
        (int32_t)(selector.x - _selector_pad), 
        (int32_t)(selector.y - _selector_pad));
    lv_obj_set_size(_selector_obj, 
        (int32_t)(selector.width + _selector_pad * 2), 
        (int32_t)(selector.height + _selector_pad * 2));

    // Update Camera (move container) + Transition Offset
    auto camera = getCameraOffset();
    lv_obj_set_pos(_menu_cont, -(int32_t)camera.x + trans_x, -(int32_t)camera.y);

    // Icon transitions
    for (size_t i = 0; i < _icon_objs.size(); i++) {
        Vector2 pos;
        if (i < _icon_transitions.size()) {
            pos = _icon_transitions[i].value();
        } else {
            const auto& kf = getOptionList()[i].keyframe;
            pos = Vector2(kf.x, kf.y);
        }
        lv_obj_set_pos(_icon_objs[i], (int32_t)pos.x, (int32_t)pos.y);
    }

    // Update Label (Apply transition offset)
    lv_obj_set_pos(_label_obj, (int32_t)(trans_x + _label_slide.value()), -5); // Y offset matches original alignment

    // Update Label Text
    int idx = getSelectedOptionIndex();
    if (idx >= 0 && idx < _settings_props.size()) {
        lv_label_set_text(_label_obj, _settings_props[idx].name.c_str());
    }
}

void MenuView::onUpdate(const uint32_t& currentTime) {
    _entry_offset.updateMs(currentTime);
    _label_slide.updateMs(currentTime);
    for (auto& t : _icon_transitions) {
        t.updateMs(currentTime);
    }
}

void MenuView::_update_camera_keyframe() {
    // Camera follow selector X to center it
    auto targetX = getSelectedKeyframe().x - (128 - _icon_w)/2; 
    getCamera().move(targetX, 0);
}

void MenuView::goNext() {
    if (_data.option_list.empty()) return;
    const uint32_t now = HAL::Millis();
    // Cooldown after a wrap: ignore any moves during the window
    if (now - _last_wrap_ms < 300) {
        return;
    }

    const int last_idx = static_cast<int>(_data.option_list.size() - 1);
    if (_data.selected_option_index == last_idx) {
        _data.selected_option_index = 0;
        _last_wrap_ms = now;
    } else {
        _data.selected_option_index++;
    }
    _data.is_changed = true;
    onGoNext();
}

void MenuView::goLast() {
    if (_data.option_list.empty()) return;
    const uint32_t now = HAL::Millis();
    if (now - _last_wrap_ms < 300) {
        return;
    }

    const int last_idx = static_cast<int>(_data.option_list.size() - 1);
    if (_data.selected_option_index == 0) {
        _data.selected_option_index = last_idx;
        _last_wrap_ms = now;
    } else {
        _data.selected_option_index--;
    }
    _data.is_changed = true;
    onGoLast();
}

void MenuView::onReadInput() {
    if (isOpening()) return;

    if (HAL::GetButton(BUTTON::BTN_RIGHT) == APP_BUTTON_STATE_CLICKED) {
        // BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE; // HAL doesn't allow reset? 
        // We might need to const_cast or add HAL method to consume event.
        // Or just use the global pointers if HAL doesn't support consume.
        // But let's check what HAL::GetButton does. It returns state.
        
        // Direct access for consumption
        if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        goNext();
    } 
    else if (HAL::GetButton(BUTTON::BTN_LEFT) == APP_BUTTON_STATE_CLICKED) {
        if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        goLast();
    }
    else if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED) {
        ESP_LOGI("MenuView", "BtnOk Clicked detected");
        if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        onClick();
    }
}

void MenuView::onClick() {
    ESP_LOGI("MenuView", "onClick");
    // Skip scale-up / open animation; trigger selection immediately
    if (_open_callback) {
        _open_callback(getSelectedOptionIndex());
    }
}

void MenuView::onOpenEnd() {
    ESP_LOGI("MenuView", "onOpenEnd");
    // Callback when animation finishes
    if (_open_callback) {
        _open_callback(getSelectedOptionIndex());
    }
}
