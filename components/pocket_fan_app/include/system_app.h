#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void system_app_init(lv_display_t* disp);
void system_app_update(void);

#ifdef __cplusplus
}
#endif
