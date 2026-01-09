#pragma once
#include <vector>
#include <string>
#include <lvgl.h>
#include "../../../ui/transitions.h"

class HealthView {
public:
    HealthView() = default;
    ~HealthView();

    void setItems(const std::vector<std::string>& items);
    void updateItem(int index, const std::string& text);
    void init();
    void playEntryAnimation();
    void update();
    void setScrollOffset(float offset);
    float getContentHeight() const;

    lv_obj_t* getScreen() const { return _screen; }

private:
    void _create_lvgl_objects();

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _list_cont = nullptr;
    std::vector<lv_obj_t*> _item_labels;
    std::vector<pocket_fan::ui::Transition2D> _item_transitions;

    std::vector<std::string> _items;
    float _scroll_offset = 0.0f;
    float _content_height = 0.0f;

    static constexpr int _item_h = 18;
    static constexpr int _item_gap = 4;
};
