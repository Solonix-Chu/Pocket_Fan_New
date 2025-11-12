// components/ui/ui_pages.c

#include "ui_priv.h"

static const char *TAG = "ui_page";

static scroll_state_t list_scroll_states[10];

static void init_select_menu_state()
{
    ui_select = 0;
    y = y_trg = 0;
    box_y = box_y_trg = 0;
    line_y = line_y_trg = 1;

    uint16_t str_width = u8g2_GetStrWidth(&u8g2, list[ui_select].select);
    if (str_width > MAX_TEXT_WIDTH) {
        box_width = box_width_trg = MAX_TEXT_WIDTH;
    } else {
        box_width = box_width_trg = str_width + x * 2;
    }

    for (int i = 0; i < list_num; i++) {
        ui_scroll_reset(&list_scroll_states[i]);
    }
}

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
        init_select_menu_state();
    }
    logo_ui_show();
}

// --- Select 页面 (API版本) ---
void select_ui_show()
{
    move_bar(&line_y, &line_y_trg);
    move(&y, &y_trg);
    move_pid(&box_y, box_y_trg, &pid_y_controller);
    move_width(&box_width, &box_width_trg, ui_select, (BtnUp->currentState == APP_BUTTON_STATE_CLICKED));

    // 绘制滚动条
    u8g2_DrawVLine(&u8g2, 126, 0, total_line_length);
    u8g2_DrawPixel(&u8g2, 125, 0);
    u8g2_DrawPixel(&u8g2, 127, 0);

    for (uint8_t i = 0; i < list_num; ++i)
    {
        int16_t current_y = 16 * i + y + 12;
        
        uint16_t str_width = u8g2_GetStrWidth(&u8g2, list[i].select);
        bool needs_scroll = str_width > MAX_TEXT_WIDTH;

        if (needs_scroll) {
            int16_t clip_y = 16 * i + y;
            u8g2_SetClipWindow(&u8g2, 0, clip_y, MAX_TEXT_WIDTH, clip_y + 16);
        }

        ui_draw_scrollable_text(&u8g2, 
                                0,
                                current_y,
                                MAX_TEXT_WIDTH,
                                list[i].select,
                                &list_scroll_states[i],
                                str_width
                               );

        if (needs_scroll) {
            u8g2_SetMaxClipWindow(&u8g2);
        }
        
        u8g2_DrawPixel(&u8g2, 125, single_line_length * (i + 1));
        u8g2_DrawPixel(&u8g2, 127, single_line_length * (i + 1));
    }

    u8g2_DrawVLine(&u8g2, 125, line_y, single_line_length - 1);
    u8g2_DrawVLine(&u8g2, 127, line_y, single_line_length - 1);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawRBox(&u8g2, 0, (int16_t)box_y, box_width, 16, 1);
    u8g2_SetDrawColor(&u8g2, 1);
}

void select_proc(void)
{
    if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED)
    {
        ESP_LOGI(TAG, "UP pressed in SELECT screen");
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        if (ui_select >= 1)
        {
            ui_select -= 1;
            line_y_trg -= single_line_length;
            if (ui_select < -(y / 16))
            {
                y_trg += 16;
            }
            else
            {
                box_y_trg -= 16;
            }
            uint16_t str_width = u8g2_GetStrWidth(&u8g2, list[ui_select].select);
            if (str_width > MAX_TEXT_WIDTH) {
                box_width_trg = MAX_TEXT_WIDTH;
            } else {
                box_width_trg = str_width + x * 2;
            }
        }
        // for (int i = 0; i < list_num; i++) {
        //     ui_scroll_reset(&list_scroll_states[i]);
        // }
    }
    else if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED)
    {
        ESP_LOGI(TAG, "DOWN pressed in SELECT screen");
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        if ((ui_select + 2) <= list_num)
        {
            ui_select += 1;
            line_y_trg += single_line_length;
            if ((ui_select + 1) > (4 - y / 16))
            {
                y_trg -= 16;
            }
            else
            {
                box_y_trg += 16;
            }
            uint16_t str_width = u8g2_GetStrWidth(&u8g2, list[ui_select].select);
            if (str_width > MAX_TEXT_WIDTH) {
                box_width_trg = MAX_TEXT_WIDTH;
            } else {
                box_width_trg = str_width + x * 2;
            }
        }
        // for (int i = 0; i < list_num; i++) {
        //     ui_scroll_reset(&list_scroll_states[i]);
        // }
    }
    else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED)
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
            init_select_menu_state();
        }
        else
        {
            animator_start(&g_popup_anim, 16.0f, -50.0f);
            ui_index = M_PID_EDIT;
        }
    }
}

void pid_edit_ui_show()
{
    char buf[20];
    int16_t y_pos = (int16_t)g_popup_anim.current;

    u8g2_DrawBox(&u8g2, 16, y_pos, 96, 31);
    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawBox(&u8g2, 17, y_pos + 1, 94, 29);
    u8g2_SetDrawColor(&u8g2, 1);

    u8g2_DrawFrame(&u8g2, 18, y_pos + 20, 60, 8);
    u8g2_DrawBox(&u8g2, 20, y_pos + 22, (uint8_t)(Kpid[pid_select] / PID_MAX * 56), 4);

    switch (pid_select)
    {
    case 0:
        u8g2_DrawStr(&u8g2, 22, y_pos + 14, "Editing Kp");
        break;
    case 1:
        u8g2_DrawStr(&u8g2, 22, y_pos + 14, "Editing Ki");
        break;
    case 2:
        u8g2_DrawStr(&u8g2, 22, y_pos + 14, "Editing Kd");
        break;
    default:
        break;
    }

    sprintf(buf, "%.2f", Kpid[pid_select]);
    u8g2_DrawStr(&u8g2, 81, y_pos + 28, buf);
}

void pid_edit_proc(void)
{
    animator_run(&g_popup_anim, &pid_y_controller);

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
    move_pid(&icon_x, icon_x_trg, &pid_x_controller);
    move(&app_y, &app_y_trg);

    for (uint8_t i = 0; i < icon_num; ++i)
    {
        u8g2_DrawXBMP(&u8g2, 46 + (int16_t)icon_x + i * ICON_SPACE, 6, 36, icon_width[i], icon_pic[i]);
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
        init_select_menu_state();
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
        init_select_menu_state();
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
            init_select_menu_state();
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
        init_select_menu_state();
    }
    about_ui_show();
}

void draw_ui_by_index(uint8_t index) {
    switch(index) {
        case M_LOGO: logo_ui_show(); break;
        case M_SELECT: select_ui_show(); break;
        case M_PID: pid_ui_show(); break;
        case M_PID_EDIT: pid_edit_ui_show(); break;
        case M_ICON: icon_ui_show(); break;
        case M_CHART: chart_ui_show(); break;
        case M_TEXT_EDIT: text_edit_ui_show(); break;
        case M_ABOUT: about_ui_show(); break;
        default: break;
    }
}