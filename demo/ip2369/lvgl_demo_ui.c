/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include "lvgl_demo_ui.h"
#include <stdio.h>

static lv_obj_t *s_label_in = NULL;
static lv_obj_t *s_label_out = NULL;
static lv_obj_t *s_label_vbat_vsys = NULL;
static lv_obj_t *s_label_ina = NULL;
static lv_obj_t *s_label_title = NULL;

void example_lvgl_demo_ui(lv_disp_t *disp)
{
    lv_obj_t *scr = lv_disp_get_scr_act(disp);
    s_label_title = lv_label_create(scr);
    lv_label_set_text(s_label_title, "IP2369 IDLE");
    lv_obj_set_pos(s_label_title, 0, 0);

    s_label_in = lv_label_create(scr);
    lv_label_set_text(s_label_in, "IN : --.- W");
    lv_obj_set_pos(s_label_in, 0, 12);

    s_label_out = lv_label_create(scr);
    lv_label_set_text(s_label_out, "OUT: --.- W");
    lv_obj_set_pos(s_label_out, 0, 24);

    s_label_vbat_vsys = lv_label_create(scr);
    lv_label_set_text(s_label_vbat_vsys, "VB/VS: --.-/--.-");
    lv_obj_set_pos(s_label_vbat_vsys, 0, 36);

    s_label_ina = lv_label_create(scr);
    lv_label_set_text(s_label_ina, "INA: --.-V --.-A");
    lv_obj_set_pos(s_label_ina, 0, 48);
}

void example_lvgl_update_power(lvgl_power_mode_t mode,
                               float input_w,
                               float output_w,
                               float vbat_v,
                               float vsys_v,
                               bool ina_valid,
                               float ina_bus_v,
                               float ina_current_a,
                               float ina_power_w)
{
    if (s_label_in == NULL || s_label_out == NULL) {
        return;
    }

    char buf[32];

    const char *mode_text = "IDLE";
    bool show_in = false;
    bool show_out = false;
    switch (mode) {
    case LVGL_POWER_MODE_INPUT:
        mode_text = "IN";
        show_in = true;
        break;
    case LVGL_POWER_MODE_OUTPUT:
        mode_text = "OUT";
        show_out = true;
        break;
    case LVGL_POWER_MODE_DUAL:
        mode_text = "DUAL";
        show_in = true;
        show_out = true;
        break;
    case LVGL_POWER_MODE_IDLE:
    default:
        mode_text = "IDLE";
        break;
    }

    if (s_label_title) {
        snprintf(buf, sizeof(buf), "IP2369 %s", mode_text);
        lv_label_set_text(s_label_title, buf);
    }

    if (show_in) {
        snprintf(buf, sizeof(buf), "IN : %5.2f W", (double)input_w);
    } else {
        snprintf(buf, sizeof(buf), "IN : --.- W");
    }
    lv_label_set_text(s_label_in, buf);

    if (show_out) {
        snprintf(buf, sizeof(buf), "OUT: %5.2f W", (double)output_w);
    } else {
        snprintf(buf, sizeof(buf), "OUT: --.- W");
    }
    lv_label_set_text(s_label_out, buf);
    if (s_label_vbat_vsys) {
        snprintf(buf, sizeof(buf), "VB/VS:%4.2f/%4.2f", (double)vbat_v, (double)vsys_v);
        lv_label_set_text(s_label_vbat_vsys, buf);
    }

    if (s_label_ina) {
        if (ina_valid) {
            snprintf(buf, sizeof(buf), "INA:%4.2fV%+5.2fA", (double)ina_bus_v, (double)ina_current_a);
        } else {
            snprintf(buf, sizeof(buf), "INA: --.-V --.-A");
        }
        lv_label_set_text(s_label_ina, buf);
    }
}
