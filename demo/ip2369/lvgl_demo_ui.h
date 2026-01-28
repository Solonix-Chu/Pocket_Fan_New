#pragma once

#include <stdbool.h>
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void example_lvgl_demo_ui(lv_disp_t *disp);
typedef enum {
    LVGL_POWER_MODE_IDLE = 0,
    LVGL_POWER_MODE_INPUT,
    LVGL_POWER_MODE_OUTPUT,
    LVGL_POWER_MODE_DUAL,
} lvgl_power_mode_t;

void example_lvgl_update_power(lvgl_power_mode_t mode,
                               float input_w,
                               float output_w,
                               float vbat_v,
                               float vsys_v,
                               bool ina_valid,
                               float ina_bus_v,
                               float ina_current_a,
                               float ina_power_w);

#ifdef __cplusplus
}
#endif
