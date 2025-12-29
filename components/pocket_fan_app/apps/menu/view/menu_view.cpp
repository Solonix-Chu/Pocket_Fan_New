#include "menu_view.h"
#include "app_button.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"
#include <esp_log.h>

MenuView::MenuView() {
    // Config
    setConfig().cameraSize = {128, 64}; // 128x64 Screen
    setConfig().renderInterval = 15;
    setConfig().readInputInterval = 20;
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
        
        // Entrance Anim: Slide from left
        getCamera().teleport(-128, 0); 
        onRender(); // Render immediately to prevent flash at 0
    }
}

void MenuView::startExitAnimation(std::function<void()> callback) {
    ESP_LOGI("MenuView", "startExitAnimation");
    // Slide out to left
    getCamera().x.onComplete(callback);
    getCamera().move(-128, 0);
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
    
    // Update Selector
    auto selector = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, 
        (int32_t)(selector.x - _selector_pad), 
        (int32_t)(selector.y - _selector_pad));
    lv_obj_set_size(_selector_obj, 
        (int32_t)(selector.width + _selector_pad * 2), 
        (int32_t)(selector.height + _selector_pad * 2));

    // Update Camera (move container)
    auto camera = getCameraOffset();
    lv_obj_set_pos(_menu_cont, -(int32_t)camera.x, -(int32_t)camera.y);

    // Update Label
    int idx = getSelectedOptionIndex();
    if (idx >= 0 && idx < _settings_props.size()) {
        lv_label_set_text(_label_obj, _settings_props[idx].name.c_str());
    }
}

void MenuView::onUpdate(const uint32_t& currentTime) {
    // Other update logic if needed
}

void MenuView::_update_camera_keyframe() {
    // Camera follow selector X to center it
    auto targetX = getSelectedKeyframe().x - (128 - _icon_w)/2; 
    getCamera().move(targetX, 0);
}

void MenuView::onReadInput() {
    if (BtnRight->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        goNext();
    } 
    else if (BtnLeft->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        goLast();
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        onClick();
    }
}

void MenuView::onClick() {
    ESP_LOGI("MenuView", "onClick");
    // Play open anim
    // Target: Full screen (adjusted for camera)
    open({0 + getCameraOffset().x, 0, 128, 64});
    
    // Speed up transition
    getSelectorPostion().x.easingOptions().duration = 0.35f;
    getSelectorPostion().y.easingOptions().duration = 0.35f;
    getSelectorShape().x.easingOptions().duration = 0.35f;
    getSelectorShape().y.easingOptions().duration = 0.35f;
}

void MenuView::onOpenEnd() {
    ESP_LOGI("MenuView", "onOpenEnd");
    // Callback when animation finishes
    if (_open_callback) {
        _open_callback(getSelectedOptionIndex());
    }
}
