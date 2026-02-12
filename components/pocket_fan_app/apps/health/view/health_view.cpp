#include "health_view.h"
#include "../../../hal/hal.h"
#include "../../../assets/assets.h"

HealthView::~HealthView() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void HealthView::setItems(const std::vector<std::string>& items) {
    _items = items;
}

void HealthView::updateItem(int index, const std::string& text) {
    if (index < 0 || index >= (int)_items.size()) return;
    _items[index] = text;
    if (index < (int)_item_labels.size() && _item_labels[index]) {
        lv_label_set_text(_item_labels[index], _items[index].c_str());
    }
}

void HealthView::init() {
    _create_lvgl_objects();
    lv_scr_load(_screen);
    playEntryAnimation();
    update();
}

void HealthView::playEntryAnimation() {
    for (size_t i = 0; i < _item_transitions.size(); i++) {
        auto& t = _item_transitions[i];
        int y = 2 + i * (_item_h + _item_gap);
        t.setDurationMs(420);
        t.setDelayMs(60 * i);
        t.setEasing(smooth_ui_toolkit::ease::ease_out_back);
        t.jumpTo(-140, y);
        t.moveTo(0, y);
    }
}

void HealthView::update() {
    if (!_screen) return;
    uint32_t now = HAL::Millis();
    for (auto& t : _item_transitions) {
        t.updateMs(now);
    }

    for (size_t i = 0; i < _item_labels.size(); i++) {
        auto pos = _item_transitions[i].value();
        lv_obj_set_pos(_item_labels[i], (int32_t)pos.x + 4, (int32_t)pos.y);
    }

    lv_obj_set_pos(_list_cont, 0, -(int32_t)_scroll_offset);
}

void HealthView::_create_lvgl_objects() {
    if (_screen) return;

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

    _item_labels.clear();
    _item_transitions.clear();
    _item_transitions.resize(_items.size());

    const lv_font_t* font = AssetPool::GetLocaleFontSmall();

    for (size_t i = 0; i < _items.size(); i++) {
        lv_obj_t* label = lv_label_create(_list_cont);
        lv_label_set_text(label, _items[i].c_str());
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_set_style_text_font(label, font, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
        lv_obj_set_width(label, 120);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
        _item_labels.push_back(label);
    }

    if (!_items.empty()) {
        _content_height = 2 + (_items.size() - 1) * (_item_h + _item_gap) + _item_h;
    } else {
        _content_height = 0;
    }
}

void HealthView::setScrollOffset(float offset) {
    _scroll_offset = offset;
    if (_list_cont) {
        lv_obj_set_pos(_list_cont, 0, -(int32_t)_scroll_offset);
    }
}

float HealthView::getContentHeight() const {
    return _content_height;
}
