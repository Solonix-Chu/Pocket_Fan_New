#include "emoji_view.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"
#include <algorithm>

EmojiView::~EmojiView()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void EmojiView::setItems(const std::vector<std::string>& items)
{
    _items = items;
}

void EmojiView::updateItem(int index, const std::string& text)
{
    if (index < 0 || index >= static_cast<int>(_items.size())) {
        return;
    }

    _items[index] = text;
    if (index < static_cast<int>(_item_labels.size()) && _item_labels[index]) {
        lv_label_set_text(_item_labels[index], text.c_str());
        _refresh_item_text_layout(index);
        if (_screen && _selected_index == index) {
            _selector_w_transition.setDelayMs(0);
            _selector_w_transition.setDurationMs(180);
            _selector_w_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
            _selector_w_transition.moveTo(static_cast<float>(_selector_target_width_for_index(index)));
        }
    }
}

void EmojiView::setSelectedIndex(int index)
{
    if (_items.empty()) {
        _selected_index = 0;
        return;
    }

    if (index < 0) {
        index = 0;
    }
    if (index >= static_cast<int>(_items.size())) {
        index = static_cast<int>(_items.size()) - 1;
    }
    const bool changed = index != _selected_index;
    _selected_index = index;
    const float target_scroll = _calc_scroll_target_for_selected();

    if (_screen && changed) {
        const float selector_y = static_cast<float>(2 + _selected_index * (_item_h + _item_gap) - _selector_pad);
        _selector_y_transition.setDelayMs(0);
        _selector_y_transition.setDurationMs(220);
        _selector_y_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
        _selector_y_transition.moveTo(selector_y);

        _selector_w_transition.setDelayMs(0);
        _selector_w_transition.setDurationMs(220);
        _selector_w_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
        _selector_w_transition.moveTo(static_cast<float>(_selector_target_width_for_index(_selected_index)));

        _scroll_transition.setDelayMs(0);
        _scroll_transition.setDurationMs(220);
        _scroll_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
        _scroll_transition.moveTo(target_scroll);
    }

    _refresh_selection_style();
}

void EmojiView::triggerTopEdgeBounce()
{
    _trigger_edge_bounce(12.0f);
}

void EmojiView::triggerBottomEdgeBounce()
{
    _trigger_edge_bounce(-12.0f);
}

void EmojiView::init()
{
    _create_lvgl_objects();

    if (!_items.empty()) {
        if (_selected_index < 0) {
            _selected_index = 0;
        }
        if (_selected_index >= static_cast<int>(_items.size())) {
            _selected_index = static_cast<int>(_items.size()) - 1;
        }
    } else {
        _selected_index = 0;
    }

    const float selector_y = static_cast<float>(2 + _selected_index * (_item_h + _item_gap) - _selector_pad);
    _selector_y_transition.setDelayMs(0);
    _selector_y_transition.setDurationMs(220);
    _selector_y_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
    _selector_y_transition.jumpTo(selector_y);

    _selector_w_transition.setDelayMs(0);
    _selector_w_transition.setDurationMs(220);
    _selector_w_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
    _selector_w_transition.jumpTo(static_cast<float>(_selector_target_width_for_index(_selected_index)));

    const float scroll_target = _calc_scroll_target_for_selected();
    _scroll_transition.setDelayMs(0);
    _scroll_transition.setDurationMs(220);
    _scroll_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
    _scroll_transition.jumpTo(scroll_target);
    _scroll_offset = _scroll_transition.value();

    _edge_bounce.setDurationMs(160);
    _edge_bounce.setEasing(smooth_ui_toolkit::ease::ease_out_back);
    _edge_bounce.jumpTo(0.0f);
    _edge_bounce_state = EdgeBounceState::Idle;
    _edge_bounce_last_ms = 0;

    lv_scr_load(_screen);
    _play_entry_animation();
    update();
}

void EmojiView::update()
{
    if (!_screen) {
        return;
    }

    const uint32_t now = HAL::Millis();
    _selector_y_transition.updateMs(now);
    _selector_w_transition.updateMs(now);
    _scroll_transition.updateMs(now);
    _edge_bounce.updateMs(now);
    for (auto& t : _item_transitions) {
        t.updateMs(now);
    }

    if (_edge_bounce_state == EdgeBounceState::Kick && _edge_bounce.isFinished()) {
        _edge_bounce.setDelayMs(0);
        _edge_bounce.setDurationMs(220);
        _edge_bounce.setEasing(smooth_ui_toolkit::ease::ease_out_back);
        _edge_bounce.moveTo(0.0f);
        _edge_bounce_state = EdgeBounceState::Return;
    } else if (_edge_bounce_state == EdgeBounceState::Return && _edge_bounce.isFinished()) {
        _edge_bounce_state = EdgeBounceState::Idle;
    }

    _scroll_offset = _scroll_transition.value();
    const int32_t bounce_y = static_cast<int32_t>(_edge_bounce.value());
    lv_obj_set_pos(_list_cont, 0, -static_cast<int32_t>(_scroll_offset) + bounce_y);
    for (size_t i = 0; i < _item_labels.size() && i < _item_transitions.size(); i++) {
        const auto pos = _item_transitions[i].value();
        lv_obj_set_pos(_item_labels[i], static_cast<int32_t>(pos.x), static_cast<int32_t>(pos.y));
    }

    _refresh_selection_style();
}

void EmojiView::_create_lvgl_objects()
{
    if (_screen) {
        return;
    }

    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    _list_cont = lv_obj_create(_screen);
    lv_obj_set_size(_list_cont, 128, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(_list_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(_list_cont, 0, 0);
    lv_obj_clear_flag(_list_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(_list_cont, 0, 0);

    _selector_obj = lv_obj_create(_list_cont);
    lv_obj_set_style_bg_color(_selector_obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(_selector_obj, 4, 0);
    lv_obj_set_style_border_width(_selector_obj, 0, 0);
    lv_obj_set_style_shadow_width(_selector_obj, 0, 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    _item_labels.clear();
    _item_selector_widths.clear();
    _item_transitions.clear();
    _item_transitions.resize(_items.size());
    _item_selector_widths.resize(_items.size(), _selector_max_w);
    const lv_font_t* font = AssetPool::GetLocaleFontSmall();
    for (size_t i = 0; i < _items.size(); i++) {
        lv_obj_t* label = lv_label_create(_list_cont);
        lv_label_set_text(label, _items[i].c_str());
        lv_obj_set_style_text_font(label, font, 0);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
        const int y = 2 + static_cast<int>(i) * (_item_h + _item_gap) + _label_y_offset;
        lv_obj_set_pos(label, _label_x, y);
        _item_labels.push_back(label);
        _refresh_item_text_layout(static_cast<int>(i));
    }

    if (!_items.empty()) {
        _content_height = static_cast<float>(2 + (_items.size() - 1) * (_item_h + _item_gap) + _item_h);
    } else {
        _content_height = 0.0f;
    }
}

void EmojiView::_play_entry_animation()
{
    for (size_t i = 0; i < _item_transitions.size(); i++) {
        auto& t = _item_transitions[i];
        const int y = 2 + static_cast<int>(i) * (_item_h + _item_gap) + _label_y_offset;
        t.setDurationMs(420);
        t.setDelayMs(60 * i);
        t.setEasing(smooth_ui_toolkit::ease::ease_out_back);
        t.jumpTo(-140, static_cast<float>(y));
        t.moveTo(static_cast<float>(_label_x), static_cast<float>(y));
    }
}

void EmojiView::_refresh_item_text_layout(int index)
{
    if (index < 0 || index >= static_cast<int>(_item_labels.size())) {
        return;
    }
    lv_obj_t* label = _item_labels[index];
    if (!label) {
        return;
    }

    lv_obj_set_width(label, LV_SIZE_CONTENT);
    lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    lv_obj_update_layout(label);
    int text_w = lv_obj_get_self_width(label);

    int visible_w = text_w;
    if (text_w > _label_clip_width) {
        visible_w = _label_clip_width;
        lv_obj_set_width(label, _label_clip_width);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    } else {
        lv_obj_set_width(label, LV_SIZE_CONTENT);
        lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    }

    const int selector_w = std::clamp(visible_w + 8, _selector_min_w, _selector_max_w);
    if (index < static_cast<int>(_item_selector_widths.size())) {
        _item_selector_widths[index] = selector_w;
    }
}

int EmojiView::_selector_target_width_for_index(int index) const
{
    if (index < 0 || index >= static_cast<int>(_item_selector_widths.size())) {
        return _selector_max_w;
    }
    return _item_selector_widths[index];
}

void EmojiView::_refresh_selection_style()
{
    if (!_selector_obj || _items.empty()) {
        return;
    }

    if (_selected_index < 0) {
        _selected_index = 0;
    }
    if (_selected_index >= static_cast<int>(_items.size())) {
        _selected_index = static_cast<int>(_items.size()) - 1;
    }

    const int y = static_cast<int>(_selector_y_transition.value());
    const int w = static_cast<int>(_selector_w_transition.value());
    lv_obj_set_pos(_selector_obj, _selector_x, y);
    lv_obj_set_size(_selector_obj, w, _item_h + _selector_pad * 2);

    for (size_t i = 0; i < _item_labels.size(); i++) {
        const bool selected = static_cast<int>(i) == _selected_index;
        lv_obj_set_style_text_color(_item_labels[i], selected ? lv_color_white() : lv_color_black(), 0);
    }
}

float EmojiView::_calc_scroll_target_for_selected() const
{
    if (_items.empty()) {
        return 0.0f;
    }

    const float selector_h = static_cast<float>(_item_h + _selector_pad * 2);
    const float top_margin = 2.0f;
    const float bottom_margin = static_cast<float>(_view_h) - selector_h - 1.0f;

    const float selected_y = static_cast<float>(2 + _selected_index * (_item_h + _item_gap));
    const float current_cam_y = _scroll_offset;

    float target_cam_y = current_cam_y;
    const float selected_screen_y = selected_y - current_cam_y;
    if (selected_screen_y < top_margin) {
        target_cam_y = selected_y - top_margin;
    } else if (selected_screen_y > bottom_margin) {
        target_cam_y = selected_y - bottom_margin;
    }

    const float extra_bottom_scroll = static_cast<float>(_selector_pad * 2 + 4);
    const float max_cam_y = (_content_height > static_cast<float>(_view_h))
        ? (_content_height - static_cast<float>(_view_h) + extra_bottom_scroll)
        : 0.0f;

    if (target_cam_y < 0.0f) {
        target_cam_y = 0.0f;
    }
    if (target_cam_y > max_cam_y) {
        target_cam_y = max_cam_y;
    }
    return target_cam_y;
}

void EmojiView::_trigger_edge_bounce(float offset)
{
    if (!_screen) {
        return;
    }
    if (_edge_bounce_state != EdgeBounceState::Idle) {
        return;
    }

    const uint32_t now = HAL::Millis();
    if (now - _edge_bounce_last_ms < _edge_bounce_cooldown_ms) {
        return;
    }
    _edge_bounce_last_ms = now;

    _edge_bounce.setDelayMs(0);
    _edge_bounce.setDurationMs(70);
    _edge_bounce.setEasing(smooth_ui_toolkit::ease::ease_out_quad);
    _edge_bounce.jumpTo(0.0f);
    _edge_bounce.moveTo(offset);
    _edge_bounce_state = EdgeBounceState::Kick;
}
