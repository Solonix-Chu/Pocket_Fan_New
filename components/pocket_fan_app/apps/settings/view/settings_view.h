#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>

using namespace smooth_ui_toolkit;

class SettingsView : public SmoothSelectorMenu {
public:
    struct SettingsItemProps {
        std::string name;
        std::function<void()> callback;
    };

    SettingsView();
    ~SettingsView();

    void init();
    void addSettingsItem(const SettingsItemProps& props);

    // Overrides
    void onRender() override;
    void onUpdate(const uint32_t& currentTime) override;
    void onReadInput() override;
    void onClick() override;
    void onOpenEnd() override;

protected:
    void _update_camera_keyframe() override;

private:
    std::vector<SettingsItemProps> _items_props;

    // LVGL Objects
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _list_cont = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    std::vector<lv_obj_t*> _item_objs;
    
    void _create_lvgl_objects();
    
    // Constants
    static constexpr int _item_h = 24;
    static constexpr int _item_gap = 4;
    static constexpr int _selector_pad = 2;

    smooth_ui_toolkit::AnimateValue _transition_offset;
};
