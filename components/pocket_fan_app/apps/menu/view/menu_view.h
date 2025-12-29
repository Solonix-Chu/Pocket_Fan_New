#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>

using namespace smooth_ui_toolkit;

class MenuView : public SmoothSelectorMenu {
public:
    struct SettingsOptionProps {
        const lv_image_dsc_t* icon; // Pointer to LVGL image descriptor
        std::string name;
    };

    MenuView();
    ~MenuView();

    void init();
    void addSettingsOption(const SettingsOptionProps& props);
    void setOpenCallback(std::function<void(int)> callback) { _open_callback = callback; }

    // Overrides
    void onRender() override;
    void onUpdate(const uint32_t& currentTime) override;
    void onReadInput() override;
    
    void onClick() override;
    void onOpenEnd() override;

protected:
    void _update_camera_keyframe() override;

private:
    std::vector<SettingsOptionProps> _settings_props;
    std::function<void(int)> _open_callback;

    // LVGL Objects
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _menu_cont = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    lv_obj_t* _panel_obj = nullptr;
    lv_obj_t* _label_obj = nullptr;
    std::vector<lv_obj_t*> _icon_objs;
    
    void _create_lvgl_objects();
    void _update_lvgl_positions();
    
    // Constants for layout
    static constexpr int _icon_w = 34;
    static constexpr int _icon_h = 26;
    static constexpr int _icon_gap = 20;
    static constexpr int _selector_pad = 5;
};
