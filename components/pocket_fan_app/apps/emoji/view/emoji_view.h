#pragma once

#include <lvgl.h>
#include <string>
#include <vector>
#include "../../../ui/transitions.h"

class EmojiView {
public:
    EmojiView() = default;
    ~EmojiView();

    void setItems(const std::vector<std::string>& items);
    void updateItem(int index, const std::string& text);
    void setSelectedIndex(int index);
    int getSelectedIndex() const { return _selected_index; }
    void triggerTopEdgeBounce();
    void triggerBottomEdgeBounce();

    void init();
    void update();

private:
    void _create_lvgl_objects();
    void _play_entry_animation();
    void _refresh_item_text_layout(int index);
    int _selector_target_width_for_index(int index) const;
    void _refresh_selection_style();
    float _calc_scroll_target_for_selected() const;
    void _trigger_edge_bounce(float offset);

private:
    enum class EdgeBounceState {
        Idle = 0,
        Kick,
        Return,
    };

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _list_cont = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    std::vector<lv_obj_t*> _item_labels;
    std::vector<int> _item_selector_widths;
    std::vector<pocket_fan::ui::Transition2D> _item_transitions;
    std::vector<std::string> _items;
    int _selected_index = 0;
    float _scroll_offset = 0.0f;
    float _content_height = 0.0f;
    pocket_fan::ui::TransitionValue _selector_y_transition;
    pocket_fan::ui::TransitionValue _selector_w_transition;
    pocket_fan::ui::TransitionValue _scroll_transition;
    pocket_fan::ui::TransitionValue _edge_bounce;
    EdgeBounceState _edge_bounce_state = EdgeBounceState::Idle;
    uint32_t _edge_bounce_last_ms = 0;

    static constexpr int _item_h = 18;
    static constexpr int _item_gap = 4;
    static constexpr int _selector_pad = 2;
    static constexpr int _label_x = 6;
    static constexpr int _label_y_offset = 2;
    static constexpr int _label_clip_width = 116;
    static constexpr int _selector_x = 2;
    static constexpr int _selector_max_w = 124;
    static constexpr int _selector_min_w = 24;
    static constexpr int _view_h = 64;
    static constexpr uint32_t _edge_bounce_cooldown_ms = 180;
};
