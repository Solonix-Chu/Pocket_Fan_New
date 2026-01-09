#include "checkbox.h"

namespace pocket_fan::ui {

Checkbox CreateCheckbox(lv_obj_t* parent, const CheckboxStyle& style) {
    Checkbox checkbox;

    checkbox.root = lv_obj_create(parent);
    lv_obj_set_size(checkbox.root, style.size, style.size);
    lv_obj_set_style_bg_opa(checkbox.root, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(checkbox.root, 0, 0);
    lv_obj_set_style_border_width(checkbox.root, style.border_width, 0);
    lv_obj_set_style_border_color(checkbox.root, style.border_color, 0);
    lv_obj_set_style_radius(checkbox.root, style.radius, 0);
    lv_obj_clear_flag(checkbox.root, LV_OBJ_FLAG_SCROLLABLE);

    checkbox.indicator = lv_obj_create(checkbox.root);
    lv_obj_set_size(checkbox.indicator, style.inner_size, style.inner_size);
    lv_obj_center(checkbox.indicator);
    lv_obj_set_style_bg_color(checkbox.indicator, style.fill_color, 0);
    lv_obj_set_style_bg_opa(checkbox.indicator, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(checkbox.indicator, 0, 0);
    lv_obj_set_style_radius(checkbox.indicator, style.inner_radius, 0);
    lv_obj_set_style_border_width(checkbox.indicator, 0, 0);
    lv_obj_clear_flag(checkbox.indicator, LV_OBJ_FLAG_SCROLLABLE);

    return checkbox;
}

void SetCheckboxState(Checkbox& checkbox, bool checked) {
    if (!checkbox.indicator) return;
    lv_obj_set_style_bg_opa(checkbox.indicator, checked ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
}

bool IsCheckboxChecked(const Checkbox& checkbox) {
    if (!checkbox.indicator) return false;
    return lv_obj_get_style_bg_opa(checkbox.indicator, LV_PART_MAIN) != LV_OPA_TRANSP;
}

} // namespace pocket_fan::ui
