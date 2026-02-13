#pragma once
#include <lvgl.h>
#include <string>
#include <vector>
#include "../../../ui/transitions.h"

class DetailView {
public:
    DetailView() = default;
    ~DetailView();

    void setItems(const std::vector<std::string>& items);
    void updateItem(int index, const std::string& text);
    void init();
    void playEntryAnimation();
    void update();
    void setScrollOffset(float offset);
    float getContentHeight() const;

private:
    void _create_lvgl_objects();

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _list_cont = nullptr;
    std::vector<lv_obj_t*> _item_labels;
    std::vector<pocket_fan::ui::Transition2D> _item_transitions;
    std::vector<std::string> _items;
    float _scroll_offset = 0.0f;
    float _content_height = 0.0f;

    static constexpr int _item_h = 16;
    static constexpr int _item_gap = 3;
};
