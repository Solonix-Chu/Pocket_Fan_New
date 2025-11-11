#include "ui_priv.h"

bool move(int16_t *a, int16_t *a_trg)
{
    if (*a < *a_trg)
    {
        *a += SPEED;
        if (*a > *a_trg)
            *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        *a -= SPEED;
        if (*a < *a_trg)
            *a = *a_trg;
    }
    else
    {
        return true;
    }
    return false;
}

bool move_icon(int16_t *a, int16_t *a_trg)
{
    if (*a < *a_trg)
    {
        *a += ICON_SPEED;
        if (*a > *a_trg)
            *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        *a -= ICON_SPEED;
        if (*a < *a_trg)
            *a = *a_trg;
    }
    else
    {
        return true;
    }
    return false;
}

bool move_width(uint8_t *a, uint8_t *a_trg, uint8_t current_select, bool is_up)
{
    // ... (原函数代码) ...
    // (注意：这里依赖 ui_index, list, pid 等全局变量)
    // (我们的新结构通过 ui_priv.h 保证了这一点)
    uint8_t len = 1; // 默认
    if (ui_index == M_SELECT)
    {
        len = abs(u8g2_GetStrWidth(&u8g2, list[current_select].select) - u8g2_GetStrWidth(&u8g2, list[is_up ? current_select + 1 : current_select - 1].select));
    }
    else if (ui_index == M_PID)
    {
        len = abs(u8g2_GetStrWidth(&u8g2, pid[current_select].select) - u8g2_GetStrWidth(&u8g2, pid[is_up ? current_select + 1 : current_select - 1].select));
    }

    if (*a < *a_trg)
    {
        uint8_t step = 16 / WIDTH_CHANGE_SPEED_FACTOR;
        uint8_t width_speed = ((len % step) == 0 ? (len / step) : (len / step + 1));
        *a += width_speed;
        if (*a > *a_trg) *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        uint8_t step = 16 / WIDTH_CHANGE_SPEED_FACTOR;
        uint8_t width_speed = ((len % step) == 0 ? (len / step) : (len / step + 1));
        *a -= width_speed;
        if (*a < *a_trg) *a = *a_trg;
    }
    else
    {
        return true;
    }
    return false;
}

bool move_bar(uint8_t *a, uint8_t *a_trg)
{
    if (*a < *a_trg)
    {
        uint8_t step = 16 / SPEED;
        uint8_t width_speed = ((single_line_length % step) == 0 ? (single_line_length / step) : (single_line_length / step + 1));
        *a += width_speed;
        if (*a > *a_trg) *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        uint8_t step = 16 / SPEED;
        uint8_t width_speed = ((single_line_length % step) == 0 ? (single_line_length / step) : (single_line_length / step + 1));
        *a -= width_speed;
        if (*a < *a_trg) *a = *a_trg;
    }
    else
    {
        return true;
    }
    return false;
}


void ui_scroll_reset(void)
{
    text_scroll_x = 0;
    text_scroll_counter = 0;
    text_scroll_pause = TEXT_SCROLL_PAUSE_FRAMES;
}

void ui_scroll_update(bool is_scrolling, uint16_t str_width)
{
    if (is_scrolling)
    {
        if (text_scroll_pause > 0) text_scroll_pause--;
        else
        {
            text_scroll_counter++;
            if (text_scroll_counter >= TEXT_SCROLL_SPEED)
            {
                text_scroll_counter = 0;
                text_scroll_x++;
                if (text_scroll_x >= (str_width + TEXT_SCROLL_SEPARATOR_WIDTH))
                {
                    text_scroll_x = 0;
                    text_scroll_pause = TEXT_SCROLL_PAUSE_FRAMES;
                }
            }
        }
    }
    else
    {
        ui_scroll_reset();
    }
}

void ui_draw_scrollable_text(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t max_width, const char *text, bool is_selected)
{
    // (注意：这里使用 u8g2，它必须是全局可访问的，就像您原代码中那样)
    uint16_t str_width = u8g2_GetStrWidth(u8g2, text);
    bool needs_scroll = (str_width > max_width);

    if (is_selected)
    {
        ui_scroll_update(needs_scroll, str_width);
        if (needs_scroll)
        {
            u8g2_DrawStr(u8g2, x - text_scroll_x, y, text);
            u8g2_DrawStr(u8g2, x - text_scroll_x + str_width + TEXT_SCROLL_SEPARATOR_WIDTH, y, text);
        }
        else
        {
            u8g2_DrawStr(u8g2, x, y, text);
        }
    }
    else
    {
        u8g2_DrawStr(u8g2, x, y, text);
    }
}


void text_edit(bool dir, uint8_t index)
{
    if (!dir) {
        if (name[index] >= 'A' && name[index] <= 'Z') {
            if (name[index] == 'A') name[index] = 'z';
            else name[index] -= 1;
        } else if (name[index] >= 'a' && name[index] <= 'z') {
            if (name[index] == 'a') name[index] = ' ';
            else name[index] -= 1;
        } else {
            name[index] = 'Z';
        }
    } else {
        if (name[index] >= 'A' && name[index] <= 'Z') {
            if (name[index] == 'Z') name[index] = ' ';
            else name[index] += 1;
        } else if (name[index] >= 'a' && name[index] <= 'z') {
            if (name[index] == 'z') name[index] = 'A';
            else name[index] += 1;
        } else {
            name[index] = 'a';
        }
    }
}

void disappear()
{
    switch (disappear_step)
    {
    case 1:
        for (uint16_t i = 0; i < buf_len; ++i) {
            if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x55;
        }
        break;
    case 2:
        for (uint16_t i = 0; i < buf_len; ++i) {
            if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0xAA;
        }
        break;
    case 3:
        for (uint16_t i = 0; i < buf_len; ++i) {
            if (i % 2 == 0) buf_ptr[i] = buf_ptr[i] & 0x00;
        }
        break;
    case 4:
        for (uint16_t i = 0; i < buf_len; ++i) {
            if (i % 2 != 0) buf_ptr[i] = buf_ptr[i] & 0x00;
        }
        break;
    default:
        ui_state = S_NONE;
        disappear_step = 0;
        break;
    }
    disappear_step++;
}