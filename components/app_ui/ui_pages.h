#ifndef UI_PAGES_H
#define UI_PAGES_H

#include <stdint.h>
#include <stdbool.h>

// 页面索引定义
typedef enum {
    M_LOGO = 0,
    M_SELECT,
    M_PID,
    M_PID_EDIT,
    M_ICON,
    M_CHART,
    M_TEXT_EDIT,
    M_ABOUT,
    M_VIDEO,
} main_state_t;

// 状态定义
typedef enum {
    S_NONE = 0,
    S_DISAPPEAR,
    S_SWITCH,
    S_MENU_TO_MENU,
    S_MENU_TO_PIC,
    S_PIC_TO_MENU,
} transition_state_t;

// 全局变量声明
extern uint8_t ui_index;
extern uint8_t ui_select;
extern transition_state_t ui_state;
extern bool g_invert_color_enabled;

// 页面函数声明
void logo_ui_show(void);
void logo_proc(void);

void select_ui_show(void);
void select_proc(void);

void pid_ui_show(void);
void pid_proc(void);

void pid_edit_ui_show(void);
void pid_edit_proc(void);

void icon_ui_show(void);
void icon_proc(void);

void chart_draw_frame(void);
void chart_ui_show(void);
void chart_proc(void);

void text_edit_ui_show(void);
void text_edit_proc(void);

void about_ui_show(void);
void about_proc(void);

void draw_ui_by_index(uint8_t index);

#endif /* UI_PAGES_H */