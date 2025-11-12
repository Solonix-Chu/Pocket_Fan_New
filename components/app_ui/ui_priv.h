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

#define SPEED 4
#define WIDTH_CHANGE_SPEED_FACTOR 8
#define ICON_SPEED 12
#define ICON_SPACE 48
// --- 长文本滚动配置 ---
// 滚动速度（每 TEXT_SCROLL_SPEED 帧滚动 TEXT_SCROLL_PIXELS_PER_FRAME 像素）
#define TEXT_SCROLL_SPEED 1               // 数值越小滚动越快
#define TEXT_SCROLL_PIXELS_PER_FRAME 1    // 数值越大滚动越快
// 滚动开始前和循环一轮后的暂停帧数
#define TEXT_SCROLL_PAUSE_FRAMES 30
// 循环滚动时，文本末尾和开头之间的像素间隔
#define TEXT_SCROLL_SEPARATOR_WIDTH 20
// 文本超过此宽度（像素）将自动开启滚动
#define MAX_TEXT_WIDTH 120
#define BLINK_SPEED 16
// --------------------

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

// Animation types for page transitions
typedef enum {
    ANIM_DITHER_FADE,
    ANIM_SLIDE_FROM_RIGHT,
    ANIM_SLIDE_FROM_LEFT,
    ANIM_SLIDE_FROM_TOP,
    ANIM_SLIDE_FROM_BOTTOM,
    ANIM_SNOW_DISSOLVE,
} transition_anim_t;

extern transition_anim_t g_current_transition;

typedef struct
{
    char *select;
} SELECT_LIST;

typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float previous_error;
} pid_controller_t;

typedef struct {
    int16_t x;
    uint16_t counter;
    uint16_t pause;
} scroll_state_t;


// --- 共享的内部状态变量 (在 ui.c 中定义) ---
extern uint8_t ui_index, ui_state;
extern uint8_t disappear_step;
extern pid_controller_t pid_y_controller;
extern pid_controller_t pid_x_controller;

// M_SELECT 状态
extern uint8_t x;
extern int16_t y, y_trg;
extern uint8_t line_y, line_y_trg;
extern uint8_t box_width, box_width_trg;
extern float box_y, box_y_trg;
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

// Generic animator for pop-up windows
typedef struct {
    float current;
    float target;
} animated_value_t;

extern animated_value_t g_popup_anim;

// M_ICON 状态
extern float icon_x, icon_x_trg;
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
bool move_pid(float *value, float target, pid_controller_t *pid);
void animator_init(animated_value_t *anim, float value);
void animator_start(animated_value_t *anim, float target, float start);
bool animator_run(animated_value_t *anim, pid_controller_t *pid);
bool move_width(uint8_t *a, uint8_t *a_trg, uint8_t current_select, bool is_up);
bool move_bar(uint8_t *a, uint8_t *a_trg);
void ui_scroll_reset(scroll_state_t *state);
void ui_scroll_update(scroll_state_t *state, bool needs_scroll, uint16_t str_width);
void ui_draw_scrollable_text(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t max_width, const char *text, scroll_state_t *state, uint16_t str_width);
void text_edit(bool dir, uint8_t index);
void run_transition(void);

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
void draw_ui_by_index(uint8_t index);
// ... (其他页面)

#endif // UI_PRIV_H