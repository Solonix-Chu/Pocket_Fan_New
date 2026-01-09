#pragma once
#include "lvgl.h"

namespace pocket_fan::ui {

struct CheckboxStyle {
    int size = 12;
    int inner_size = 6;
    int border_width = 2;
    int radius = 1;
    int inner_radius = 0;
    lv_color_t border_color = lv_color_black();
    lv_color_t fill_color = lv_color_black();
};

struct Checkbox {
    lv_obj_t* root = nullptr;
    lv_obj_t* indicator = nullptr;
};

Checkbox CreateCheckbox(lv_obj_t* parent, const CheckboxStyle& style = {});
void SetCheckboxState(Checkbox& checkbox, bool checked);
bool IsCheckboxChecked(const Checkbox& checkbox);

} // namespace pocket_fan::ui
