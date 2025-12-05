#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

void app_init(lv_display_t* disp);
void app_update(void);

#ifdef __cplusplus
}
#endif
