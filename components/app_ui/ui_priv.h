// components/ui/ui_priv.h

#ifndef UI_PRIV_H
#define UI_PRIV_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

// ESP-IDF 和硬件驱动
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"

// 假设这些是您项目中的其他组件或驱动
// 如果它们在 main/include 中，请确保构建路径正确
#include "usr_lcd.h"
#include "app_button.h"

// 滚动配置
#define SPEED 4
#define WIDTH_CHANGE_SPEED_FACTOR 8
#define ICON_SPEED 12
#define ICON_SPACE 48
#define TEXT_SCROLL_SPEED 1
#define TEXT_SCROLL_PAUSE_FRAMES 30
#define TEXT_SCROLL_SEPARATOR_WIDTH 20
#define MAX_TEXT_WIDTH 120
#define BLINK_SPEED 16

// 主状态
enum
{
    M_LOGO,
    M_SELECT,
    M_PID,
    M_PID_EDIT,
    M_ICON,
    M_CHART,
    M_TEXT_EDIT,
    M_VIDEO,
    M_ABOUT,
};

// 过渡状态
enum
{
    S_NONE,
    S_DISAPPEAR,
    S_SWITCH,
    S_MENU_TO_MENU,
    S_MENU_TO_PIC,
    S_PIC_TO_MENU,
};

typedef struct
{
    char *select;
} SELECT_LIST;


// --- 共享的内部状态变量 (在 ui.c 中定义) ---
extern uint8_t ui_index, ui_state;
extern uint8_t disappear_step;

// M_SELECT 状态
extern uint8_t x;
extern int16_t y, y_trg;
extern uint8_t line_y, line_y_trg;
extern uint8_t box_width, box_width_trg;
extern int16_t box_y, box_y_trg;
extern int8_t ui_select;
extern uint8_t list_num;
extern uint8_t single_line_length;
extern uint8_t total_line_length;

// M_PID 状态
extern uint8_t pid_line_y, pid_line_y_trg;
extern uint8_t pid_box_width, pid_box_width_trg;
extern int16_t pid_box_y, pid_box_y_trg;
extern int8_t pid_select;
extern uint8_t pid_num;

// M_ICON 状态
extern int16_t icon_x, icon_x_trg;
extern int16_t app_y, app_y_trg;
extern int8_t icon_select;
extern uint8_t icon_num;

// M_CHART 状态
extern float angle, angle_last;
extern uint8_t chart_x;
extern bool frame_is_drawed;

// M_TEXT_EDIT 状态
extern uint8_t edit_index;
extern bool edit_flag;
extern uint8_t blink_flag;
extern const uint8_t name_len;

// 滚动状态
extern int16_t text_scroll_x;
extern uint8_t text_scroll_counter;
extern uint16_t text_scroll_pause;

// LCD 缓冲区
extern uint8_t *buf_ptr;
extern uint16_t buf_len;

// --- 资源 (在 ui_assets.c 中定义) ---
extern const uint8_t icon_pic[][200];
extern const uint8_t icon_width[];
extern const uint8_t LOGO[];
extern const float PID_MAX;
extern float Kpid[3];
extern SELECT_LIST pid[];
extern SELECT_LIST list[];
extern SELECT_LIST icon[];
extern char name[];

// --- 工具函数 (在 ui_utils.c 中定义) ---
bool move(int16_t *a, int16_t *a_trg);
bool move_icon(int16_t *a, int16_t *a_trg);
bool move_width(uint8_t *a, uint8_t *a_trg, uint8_t current_select, bool is_up);
bool move_bar(uint8_t *a, uint8_t *a_trg);
void ui_scroll_reset(void);
void ui_scroll_update(bool is_scrolling, uint16_t str_width);
void ui_draw_scrollable_text(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t max_width, const char *text, bool is_selected);
void text_edit(bool dir, uint8_t index);
void disappear(void);

// --- 页面函数 (在 ui_pages.c 中定义) ---
void logo_proc(void);
void logo_ui_show(void);
void select_proc(void);
void select_ui_show(void);
void pid_proc(void);
void pid_ui_show(void);
void pid_edit_proc(void);
void icon_proc(void);
void icon_ui_show(void);
void chart_proc(void);
void chart_draw_frame(void);
void chart_ui_show(void);
void text_edit_proc(void);
void text_edit_ui_show(void);
void about_proc(void);
void about_ui_show(void);
// ... (其他页面)

#endif // UI_PRIV_H