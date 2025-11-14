/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl.h"
#include "include/smooth_ui_demo.h"

void example_lvgl_demo_ui(lv_display_t *disp)
{
    smooth_ui_demo(disp);

    // lv_obj_t *scr = lv_display_get_screen_active(disp);
    // lv_obj_t *label = lv_label_create(scr);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    // lv_label_set_text(label, "Hello Espressif, Hello LVGL 1.");
    // /* Size of the screen (if you use rotation 90 or 270, please set disp->driver->ver_res) */
    // lv_obj_set_width(label, lv_disp_get_hor_res(disp));
    // lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    // //创建另一个label
    // lv_obj_t *label2 = lv_label_create(scr);
    // lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    // lv_label_set_text(label2, "Hello Espressif, Hello LVGL 3!");
    // lv_obj_set_width(label2, lv_disp_get_hor_res(disp));
    // lv_obj_align(label2, LV_ALIGN_TOP_MID, 0, 23);

    // //创建另一个label
    // lv_obj_t *label3 = lv_label_create(scr);
    // lv_label_set_long_mode(label3, LV_LABEL_LONG_SCROLL_CIRCULAR); /* Circular scroll */
    // lv_label_set_text(label3, "Hello Espressif, Hello LVGL 2!");
    // lv_obj_set_width(label3, lv_disp_get_hor_res(disp));
    // lv_obj_align(label3, LV_ALIGN_TOP_MID, 0, 46); 
}
