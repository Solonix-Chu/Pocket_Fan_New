#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>
#include "../../../ui/transitions.h"
#include "../../../ui/checkbox.h"

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
    void updateItemText(int index, const std::string& text);
    void playEntryAnimation();
    void setInitialIndex(int index) { _initial_index = index; }
    void setSkipEntryAnimation(bool skip) { _skip_entry_anim = skip; }

    // Popup management
    void showBrightnessPopup(int initialValue);
    void updateBrightnessPopup(int value);
    void hideBrightnessPopup();
    bool isPopupActive() const { return _is_popup_active; }

    // Generic popup helpers
    void showValuePopup(const std::string& labelText, int initialValue, int minValue, int maxValue);
    void updateValuePopup(const std::string& labelText, int value);
    void showMessagePopup(const std::string& message);
    void updateMessagePopup(const std::string& message);
    void hidePopup();

    // Overrides
    void onRender() override;
    void onUpdate(const uint32_t& currentTime) override;
    void onReadInput() override;
    void onClick() override;
    void onOpenEnd() override;
    void goNext() override;
    void goLast() override;

protected:
    void _update_camera_keyframe() override;
    void _trigger_edge_bounce(float offset);

private:
    enum class EdgeBounceState {
        Idle = 0,
        Kick,
        Return,
    };

    std::vector<SettingsItemProps> _items_props;

    // LVGL Objects
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _list_cont = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    std::vector<lv_obj_t*> _item_objs;
    std::vector<pocket_fan::ui::Checkbox> _checkbox_objs;
    
    // Popup objects
    lv_obj_t* _popup_cont = nullptr;
    lv_obj_t* _popup_bar = nullptr;
    lv_obj_t* _popup_label = nullptr;
    bool _is_popup_active = false;
    
    void _create_lvgl_objects();
    void _refresh_item_text_layout(int index);
    
    // Constants
    static constexpr int _item_h = 18;
    static constexpr int _item_gap = 4;
    static constexpr int _selector_pad = 2;

    int _initial_index = 0;
    bool _skip_entry_anim = false;

    // Transition animation values
    pocket_fan::ui::TransitionValue _transition_offset;
    std::vector<pocket_fan::ui::Transition2D> _item_transitions;
    pocket_fan::ui::TransitionValue _popup_transition;
    pocket_fan::ui::TransitionValue _edge_bounce;
    EdgeBounceState _edge_bounce_state = EdgeBounceState::Idle;
    uint32_t _edge_bounce_last_ms = 0;
    static constexpr uint32_t _edge_bounce_cooldown_ms = 180;
    bool _popup_closing = false;
    uint32_t _last_wrap_ms = 0;
};
