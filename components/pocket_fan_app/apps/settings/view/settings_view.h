#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>
#include "../../../ui/transitions.h"

using namespace smooth_ui_toolkit;

class SettingsView : public SmoothSelectorMenu {
public:
    struct SettingsItemProps {
        std::string name;
        bool has_checkbox = false;
        bool checked = false;
        std::function<void()> callback;
    };

    SettingsView();
    ~SettingsView();

    void init();
    void addSettingsItem(const SettingsItemProps& props);
    void updateItemValue(int index, bool checked);
    void playEntryAnimation();

    // Popup management
    void showBrightnessPopup(int initialValue);
    void updateBrightnessPopup(int value);
    void hideBrightnessPopup();
    bool isPopupActive() const { return _is_popup_active; }

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
    std::vector<lv_obj_t*> _checkbox_objs;
    
    // Popup objects
    lv_obj_t* _popup_cont = nullptr;
    lv_obj_t* _popup_bar = nullptr;
    lv_obj_t* _popup_label = nullptr;
    bool _is_popup_active = false;
    
    void _create_lvgl_objects();
    
    // Constants
    static constexpr int _item_h = 18;
    static constexpr int _item_gap = 4;
    static constexpr int _selector_pad = 2;

    // Transition animation values
    pocket_fan::ui::TransitionValue _transition_offset;
    std::vector<pocket_fan::ui::Transition2D> _item_transitions;
    pocket_fan::ui::TransitionValue _popup_transition;
    bool _popup_closing = false;
};
