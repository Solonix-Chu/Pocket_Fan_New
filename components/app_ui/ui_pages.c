// components/ui/ui_pages.c

#include "ui_priv.h"

static const char *TAG = "ui_page";

// --- Logo 页面 ---
void logo_ui_show()
{
    u8g2_DrawXBMP(&u8g2, 0, 0, 128, 64, LOGO);
}

void logo_proc(void)
{
    if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        ESP_LOGI(TAG, "OK pressed in LOGO screen, changing to SELECT");
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ui_state = S_DISAPPEAR;
        ui_index = M_SELECT;
    }
    logo_ui_show();
}


// --- Select 页面 (API版本) ---
void select_ui_show()
{
    move_bar(&line_y, &line_y_trg);
    move(&y, &y_trg);
    move(&box_y, &box_y_trg);
    move_width(&box_width, &box_width_trg, ui_select, (BtnUp->currentState == APP_BUTTON_STATE_CLICKED));

    // 绘制滚动条
    u8g2_DrawVLine(&u8g2, 126, 0, total_line_length);
    u8g2_DrawPixel(&u8g2, 125, 0);
    u8g2_DrawPixel(&u8g2, 127, 0);

    for (uint8_t i = 0; i < list_num; ++i)
    {
        int16_t current_y = 16 * i + y + 12;
        bool is_selected = (i == ui_select);

        if (is_selected)
        {
            // 1. 设置裁剪窗口
            u8g2_SetClipWindow(&u8g2, 0, box_y, MAX_TEXT_WIDTH, box_y + 16);
        }

        // 2. 调用API函数进行绘制
        // (假设菜单从x=0开始)
        ui_draw_scrollable_text(&u8g2, 
                                0,                     // 文本X坐标
                                current_y,             // 文本Y坐标
                                MAX_TEXT_WIDTH,        // 允许的最大宽度
                                list[i].select,        // 文本内容
                                is_selected            // 是否为选中项
                               );

        if (is_selected)
        {
            // 3. 移除裁剪窗口
            u8g2_SetMaxClipWindow(&u8g2);
        }
        
        u8g2_DrawPixel(&u8g2, 125, single_line_length * (i + 1));
        u8g2_DrawPixel(&u8g2, 127, single_line_length * (i + 1));
    }

    u8g2_DrawVLine(&u8g2, 125, line_y, single_line_length - 1);
    u8g2_DrawVLine(&u8g2, 127, line_y, single_line_length - 1);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawRBox(&u8g2, 0, box_y, box_width, 16, 1);
    u8g2_SetDrawColor(&u8g2, 1);
}

void select_proc(void)
{
    // (您的按键逻辑，例如)
    if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED)
    {
        if (ui_select > 0)
        {
            ui_select--;
            line_y_trg -= single_line_length;
            y_trg += 16;
            box_width_trg = u8g2_GetStrWidth(&u8g2, list[ui_select].select);
            box_y_trg -= 16;
            ui_scroll_reset(); // 切换时重置滚动！
        }
    }
    
    if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED)
    {
         if (ui_select < list_num - 1)
        {
            ui_select++;
            line_y_trg += single_line_length;
            y_trg -= 16;
            box_width_trg = u8g2_GetStrWidth(&u8g2, list[ui_select].select);
            box_y_trg += 16;
            ui_scroll_reset(); // 切换时重置滚动！
        }
    }

    if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        ESP_LOGI(TAG, "OK pressed in SELECT screen, item %d", ui_select);
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        switch (ui_select)
        {
        case 0: // return
            ui_state = S_DISAPPEAR;
            ui_index = M_LOGO;
            break;
        case 1: // pid
            ui_state = S_DISAPPEAR;
            ui_index = M_PID;
            break;
        case 2: // icon
            ui_state = S_DISAPPEAR;
            ui_index = M_ICON;
            break;
        case 3: // chart
            ui_state = S_DISAPPEAR;
            ui_index = M_CHART;
            break;
        case 4: // textedit
            ui_state = S_DISAPPEAR;
            ui_index = M_TEXT_EDIT;
            break;
        case 6: // about
            ui_state = S_DISAPPEAR;
            ui_index = M_ABOUT;
            break;
        default:
            break;
        }
    }
    select_ui_show();
}


// --- PID 页面 ---
void pid_ui_show()
{
    move_bar(&pid_line_y, &pid_line_y_trg);
    move(&pid_box_y, &pid_box_y_trg);
    move_width(&pid_box_width, &pid_box_width_trg, pid_select, (BtnUp->currentState == APP_BUTTON_STATE_CLICKED));
    u8g2_DrawVLine(&u8g2, 126, 0, 61);
    u8g2_DrawPixel(&u8g2, 125, 0);
    u8g2_DrawPixel(&u8g2, 127, 0);
    for (uint8_t i = 0; i < pid_num; ++i)
    {
        u8g2_DrawStr(&u8g2, x, 16 * i + 12, pid[i].select);
        u8g2_DrawPixel(&u8g2, 125, 15 * (i + 1));
        u8g2_DrawPixel(&u8g2, 127, 15 * (i + 1));
    }

    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawRBox(&u8g2, 0, pid_box_y, pid_box_width, 16, 1);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawVLine(&u8g2, 125, pid_line_y, 14);
    u8g2_DrawVLine(&u8g2, 127, pid_line_y, 14);
}

void pid_proc()
{
    pid_ui_show();
    if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (pid_select != 0)
        {
            pid_select -= 1;
            pid_line_y_trg -= 15;
            pid_box_y_trg -= 16;
            pid_box_width_trg = u8g2_GetStrWidth(&u8g2, pid[pid_select].select) + x * 2;
        }
    }
    else if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (pid_select != 3)
        {
            pid_select += 1;
            pid_line_y_trg += 15;
            pid_box_y_trg += 16;
            pid_box_width_trg = u8g2_GetStrWidth(&u8g2, pid[pid_select].select) + x * 2;
        }
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (pid_select == 3)
        {
            ui_index = M_SELECT;
            ui_state = S_DISAPPEAR;
            pid_select = 0;
            pid_line_y = pid_line_y_trg = 1;
            pid_box_y = pid_box_y_trg = 0;
            pid_box_width = pid_box_width_trg = u8g2_GetStrWidth(&u8g2, pid[pid_select].select) + x * 2;
        }
        else
        {
            ui_index = M_PID_EDIT;
        }
    }
}

void pid_edit_ui_show()
{
    char buf[20];
    u8g2_DrawBox(&u8g2, 16, 16, 96, 31);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawBox(&u8g2, 17, 17, 94, 29);
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_DrawFrame(&u8g2, 18, 36, 60, 8);
    u8g2_DrawBox(&u8g2, 20, 38, (uint8_t)(Kpid[pid_select] / PID_MAX * 56), 4);

    switch (pid_select)
    {
    case 0:
        u8g2_DrawStr(&u8g2, 22, 30, "Editing Kp");
        break;
    case 1:
        u8g2_DrawStr(&u8g2, 22, 30, "Editing Ki");
        break;
    case 2:
        u8g2_DrawStr(&u8g2, 22, 30, "Editing Kd");
        break;
    default:
        break;
    }

    sprintf(buf, "%.2f", Kpid[pid_select]);
    u8g2_DrawStr(&u8g2, 81, 44, buf);
}

void pid_edit_proc(void)
{
    if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (Kpid[pid_select] < PID_MAX)
            Kpid[pid_select] += 0.01;
    }
    else if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (Kpid[pid_select] > 0)
            Kpid[pid_select] -= 0.01;
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ui_index = M_PID;
    }

    pid_ui_show();
    for (uint16_t i = 0; i < buf_len; ++i)
    {
        buf_ptr[i] = buf_ptr[i] & (i % 2 == 0 ? 0x55 : 0xAA);
    }
    pid_edit_ui_show();
}

void icon_ui_show(void)
{
    move_icon(&icon_x, &icon_x_trg);
    move(&app_y, &app_y_trg);

    for (uint8_t i = 0; i < icon_num; ++i)
    {
        u8g2_DrawXBMP(&u8g2, 46 + icon_x + i * ICON_SPACE, 6, 36, icon_width[i], icon_pic[i]);
        u8g2_SetClipWindow(&u8g2, 0, 48, 128, 64);
        u8g2_DrawStr(&u8g2, (128 - u8g2_GetStrWidth(&u8g2, icon[i].select)) / 2, 62 - app_y + i * 16, icon[i].select);
        u8g2_SetMaxClipWindow(&u8g2);
    }
}

void icon_proc(void)
{
    icon_ui_show();
    if (BtnLeft->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (icon_select != 0)
        {
            icon_select -= 1;
            app_y_trg -= 16;
            icon_x_trg += ICON_SPACE;
        }
    }
    else if (BtnRight->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (icon_select != (icon_num - 1))
        {
            icon_select += 1;
            app_y_trg += 16;
            icon_x_trg -= ICON_SPACE;
        }
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ui_state = S_DISAPPEAR;
        ui_index = M_SELECT;
        icon_select = 0;
        icon_x = icon_x_trg = 0;
        app_y = app_y_trg = 0;
    }
}

// --- Chart 页面 ---
void chart_draw_frame()
{
    u8g2_DrawStr(&u8g2, 4, 12, "Real time angle :");
    u8g2_DrawRBox(&u8g2, 4, 18, 120, 46, 8);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawHLine(&u8g2, 10, 58, 108);
    u8g2_DrawVLine(&u8g2, 10, 24, 34);
    // Arrow
    u8g2_DrawPixel(&u8g2, 7, 27);
    u8g2_DrawPixel(&u8g2, 8, 26);
    u8g2_DrawPixel(&u8g2, 9, 25);

    u8g2_DrawPixel(&u8g2, 116, 59);
    u8g2_DrawPixel(&u8g2, 115, 60);
    u8g2_DrawPixel(&u8g2, 114, 61);
    u8g2_SetDrawColor(&u8g2, 1);
}

void chart_ui_show()
{
    char buf[20];
    if (!frame_is_drawed)
    {
        u8g2_ClearBuffer(&u8g2);
        chart_draw_frame();
        angle_last = 20.00 + (float)(1024 / 2) / 100.00; // Dummy value
        frame_is_drawed = true;
    }

    u8g2_DrawBox(&u8g2, 96, 0, 30, 14);

    u8g2_DrawVLine(&u8g2, chart_x + 10, 59, 3);
    if (chart_x == 100)
        chart_x = 0;

    u8g2_DrawVLine(&u8g2, chart_x + 11, 24, 34);
    u8g2_DrawVLine(&u8g2, chart_x + 12, 24, 34);
    u8g2_DrawVLine(&u8g2, chart_x + 13, 24, 34);
    u8g2_DrawVLine(&u8g2, chart_x + 14, 24, 34);

    u8g2_SetDrawColor(&u8g2, 2);
    angle = 20.00 + (float)(1024 / 2) / 100.00; // Dummy value
    u8g2_DrawLine(&u8g2, chart_x + 11, 58 - (int)angle_last / 2, chart_x + 12, 58 - (int)angle / 2);
    u8g2_DrawVLine(&u8g2, chart_x + 12, 59, 3);
    angle_last = angle;
    chart_x += 2;
    u8g2_DrawBox(&u8g2, 96, 0, 30, 14);
    u8g2_SetDrawColor(&u8g2, 1);

    sprintf(buf, "%.2f", angle);
    u8g2_DrawStr(&u8g2, 96, 12, buf);
}

void chart_proc()
{
    chart_ui_show();
    if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ui_state = S_DISAPPEAR;
        ui_index = M_SELECT;
        frame_is_drawed = false;
        chart_x = 0;
    }
}

// --- Text Edit 页面 ---
void text_edit_ui_show()
{
    u8g2_DrawRFrame(&u8g2, 4, 6, 120, 52, 8);
    u8g2_DrawStr(&u8g2, (128 - u8g2_GetStrWidth(&u8g2, "--Text Editor--")) / 2, 20, "--Text Editor--");
    u8g2_DrawStr(&u8g2, 10, 38, name);
    u8g2_DrawStr(&u8g2, 80, 50, "-Return");

    uint8_t box_x_calc = 9;

    if (edit_index < name_len)
    {
        if (blink_flag < BLINK_SPEED / 2)
        {
            for (uint8_t i = 0; i < edit_index; ++i)
            {
                char temp[2] = {name[i], '\0'};
                box_x_calc += u8g2_GetStrWidth(&u8g2, temp);
                if (name[i] != ' ')
                {
                    box_x_calc++;
                }
            }
            char temp[2] = {name[edit_index], '\0'};
            u8g2_SetDrawColor(&u8g2, 2);
            u8g2_DrawBox(&u8g2, box_x_calc, 26, u8g2_GetStrWidth(&u8g2, temp) + 2, 16);
            u8g2_SetDrawColor(&u8g2, 1);
        }
    }
    else
    {
        u8g2_SetDrawColor(&u8g2, 2);
        u8g2_DrawRBox(&u8g2, 78, 38, u8g2_GetStrWidth(&u8g2, "-Return") + 4, 16, 1);
        u8g2_SetDrawColor(&u8g2, 1);
    }

    if (edit_flag)
    {
        if (blink_flag < BLINK_SPEED)
        {
            blink_flag += 1;
        }
        else
        {
            blink_flag = 0;
        }
    }
    else
    {
        blink_flag = 0;
    }
}

void text_edit_proc()
{
    text_edit_ui_show();
    if (BtnLeft->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (edit_flag)
        {
            text_edit(false, edit_index);
        }
        else
        {
            if (edit_index == 0)
            {
                edit_index = name_len;
            }
            else
            {
                edit_index -= 1;
            }
        }
    }
    else if (BtnRight->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (edit_flag)
        {
            text_edit(true, edit_index);
        }
        else
        {
            if (edit_index == name_len)
            {
                edit_index = 0;
            }
            else
            {
                edit_index += 1;
            }
        }
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (edit_index == name_len)
        {
            ui_state = S_DISAPPEAR;
            ui_index = M_SELECT;
            edit_index = 0;
        }
        else
        {
            edit_flag = !edit_flag;
        }
    }
}

// --- About 页面 ---
void about_ui_show()
{
    u8g2_DrawStr(&u8g2, 2, 12, "MCU : ESP32");
    u8g2_DrawStr(&u8g2, 2, 28, "FLASH : 4MB");
    u8g2_DrawStr(&u8g2, 2, 44, "SRAM : 520KB");
    u8g2_DrawStr(&u8g2, 2, 60, "RTC SRAM : 16KB");
}

void about_proc()
{
    if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
    {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        ui_state = S_DISAPPEAR;
        ui_index = M_SELECT;
    }
    about_ui_show();
}