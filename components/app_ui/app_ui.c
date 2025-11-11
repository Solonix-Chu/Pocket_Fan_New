// components/ui/ui.c

#include "app_ui.h"        // 公共 API
#include "ui_priv.h"   // 私有定义

static const char *TAG = "app_ui";

// --- 共享的内部状态变量定义 ---
uint8_t ui_index, ui_state;
uint8_t disappear_step = 1;

// M_SELECT 状态
uint8_t x;
int16_t y, y_trg;
uint8_t line_y, line_y_trg;
uint8_t box_width, box_width_trg;
int16_t box_y, box_y_trg;
int8_t ui_select;
// uint8_t list_num;
uint8_t single_line_length;
uint8_t total_line_length;

// M_PID 状态
uint8_t pid_line_y, pid_line_y_trg;
uint8_t pid_box_width, pid_box_width_trg;
int16_t pid_box_y, pid_box_y_trg;
int8_t pid_select;
// uint8_t pid_num;

// M_ICON 状态
int16_t icon_x, icon_x_trg;
int16_t app_y, app_y_trg;
int8_t icon_select;
// uint8_t icon_num;

// M_CHART 状态
float angle, angle_last;
uint8_t chart_x;
bool frame_is_drawed = false;

// M_TEXT_EDIT 状态
uint8_t edit_index = 0;
bool edit_flag = false;
uint8_t blink_flag;
const uint8_t name_len = 12;

// 滚动状态
int16_t text_scroll_x = 0;
uint8_t text_scroll_counter = 0;
uint16_t text_scroll_pause = 0;

// LCD 缓冲区
uint8_t *buf_ptr;
uint16_t buf_len;

// --- 公共 API 实现 ---

/**
 * @brief 初始化UI组件
 */
void ui_init(void)
{
    // 初始化 u8g2 缓冲区指针 (在原文件中有使用)
    buf_ptr = u8g2_GetBufferPtr(&u8g2);
    buf_len = u8g2_GetBufferTileHeight(&u8g2) * u8g2_GetBufferTileWidth(&u8g2) * 8;
    
    // // 初始化菜单状态
    // list_num = sizeof(list) / sizeof(SELECT_LIST);
    // pid_num = sizeof(pid) / sizeof(SELECT_LIST);
    // icon_num = sizeof(icon) / sizeof(SELECT_LIST);

    // 计算滚动条
    u8g2_SetFont(&u8g2, u8g2_font_wqy12_t_gb2312); // A font that supports Chinese
    single_line_length = 64 / list_num;
    total_line_length = single_line_length * list_num;

    // 设置初始状态
    ui_index = M_LOGO; // 假设从LOGO开始
    ui_state = S_NONE;
    ui_select = 0;
    
    // ... 其他所有变量的默认值初始化 ...
    y = y_trg = 0;
    line_y = line_y_trg = 0;
    box_width = box_width_trg = u8g2_GetStrWidth(&u8g2, list[0].select);
    box_y = box_y_trg = 0;

    ESP_LOGI(TAG, "UI Component Initialized.");
}


/**
 * @brief UI主处理函数
 */
void ui_proc(void)
{
    switch (ui_state)
    {
    case S_NONE:
        if (ui_index != M_CHART)
            u8g2_ClearBuffer(&u8g2);
        switch (ui_index)
        {
        case M_LOGO:
            logo_proc();
            break;
        case M_SELECT:
            select_proc();
            break;
        case M_PID:
            pid_proc();
            break;
        case M_ICON:
            icon_proc();
            break;
        case M_CHART:
            chart_proc();
            break;
        case M_TEXT_EDIT:
            text_edit_proc();
            break;
        case M_PID_EDIT:
            pid_edit_proc();
            break;
        case M_ABOUT:
            about_proc();
            break;
        default:
            break;
        }
        break;
    case S_DISAPPEAR:
        disappear();
        break;
    default:
        break;
    }
    u8g2_SendBuffer(&u8g2);
}

void ui_task(void *pvParameters)
{
    while (1)
    {
        ui_proc();           // Process UI and input events
        vTaskDelay(pdMS_TO_TICKS(10)); // UI update rate
    }
}