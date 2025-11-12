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
    if (*a < *a_trg)
    {
        uint8_t step = 16 / WIDTH_CHANGE_SPEED_FACTOR;
        uint8_t len;
        if (ui_index == M_SELECT)
        {
            len = abs(u8g2_GetStrWidth(&u8g2, list[current_select].select) - u8g2_GetStrWidth(&u8g2, list[is_up ? current_select + 1 : current_select - 1].select));
        }
        else if (ui_index == M_PID)
        {
            len = abs(u8g2_GetStrWidth(&u8g2, pid[current_select].select) - u8g2_GetStrWidth(&u8g2, pid[is_up ? current_select + 1 : current_select - 1].select));
        }
        else {
            len = 1;
        }
        uint8_t width_speed = ((len % step) == 0 ? (len / step) : (len / step + 1));
        *a += width_speed;
        if (*a > *a_trg)
            *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        uint8_t step = 16 / WIDTH_CHANGE_SPEED_FACTOR;
        uint8_t len;
        if (ui_index == M_SELECT)
        {
            len = abs(u8g2_GetStrWidth(&u8g2, list[current_select].select) - u8g2_GetStrWidth(&u8g2, list[is_up ? current_select + 1 : current_select - 1].select));
        }
        else if (ui_index == M_PID)
        {
            len = abs(u8g2_GetStrWidth(&u8g2, pid[current_select].select) - u8g2_GetStrWidth(&u8g2, pid[is_up ? current_select + 1 : current_select - 1].select));
        }
        else {
            len = 1;
        }
        uint8_t width_speed = ((len % step) == 0 ? (len / step) : (len / step + 1));
        *a -= width_speed;
        if (*a < *a_trg)
            *a = *a_trg;
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
        if (*a > *a_trg)
            *a = *a_trg;
    }
    else if (*a > *a_trg)
    {
        uint8_t step = 16 / SPEED;
        uint8_t width_speed = ((single_line_length % step) == 0 ? (single_line_length / step) : (single_line_length / step + 1));
        *a -= width_speed;
        if (*a < *a_trg)
            *a = *a_trg;
    }
    else
    {
        return true;
    }
    return false;
}


void ui_scroll_reset(scroll_state_t *state)
{
    state->x = 0;
    state->counter = 0;
    state->pause = TEXT_SCROLL_PAUSE_FRAMES;
}

void ui_scroll_update(scroll_state_t *state, bool needs_scroll, uint16_t str_width)
{
    if (needs_scroll)
    {
        if (state->pause > 0) state->pause--;
        else
        {
            state->counter++;
            if (state->counter >= TEXT_SCROLL_SPEED)
            {
                state->counter = 0;
                state->x += TEXT_SCROLL_PIXELS_PER_FRAME;
                if (state->x >= (str_width + TEXT_SCROLL_SEPARATOR_WIDTH))
                {
                    state->x -= (str_width + TEXT_SCROLL_SEPARATOR_WIDTH);
                }
            }
        }
    }
    else
    {
        ui_scroll_reset(state);
    }
}

void ui_draw_scrollable_text(u8g2_t *u8g2, int16_t x, int16_t y, uint16_t max_width, const char *text, scroll_state_t *state, uint16_t str_width)
{
    bool needs_scroll = (str_width > max_width);

    ui_scroll_update(state, needs_scroll, str_width);

    if (needs_scroll)
    {
        u8g2_DrawStr(u8g2, x - state->x, y, text);
        u8g2_DrawStr(u8g2, x - state->x + str_width + TEXT_SCROLL_SEPARATOR_WIDTH, y, text);
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

void run_transition()
{
    // Define animation step counts
    const int slide_steps = 16;
    const int dither_steps = 5 * 3; // 5 levels, 3 frames each
    const int snow_steps = 10; // Decreased from 20 for faster animation

    int animation_end_step = 0;

    // On the first frame of any animation, perform setup
    if (disappear_step == 1) {
        switch (g_current_transition) {
            case ANIM_DITHER_FADE:
            case ANIM_SNOW_DISSOLVE:
                // These work on the old buffer, so do nothing to it
                break;
            case ANIM_SLIDE_FROM_RIGHT:
            case ANIM_SLIDE_FROM_LEFT:
            case ANIM_SLIDE_FROM_TOP:
            case ANIM_SLIDE_FROM_BOTTOM:
                // These draw the new screen, so clear the buffer first
                u8g2_ClearBuffer(&u8g2);
                break;
        }
    }

    // Run the animation for the current step
    switch (g_current_transition)
    {
        case ANIM_DITHER_FADE: {
            animation_end_step = dither_steps;
            if (disappear_step > animation_end_step) break;

            // Helper function for dithering fade
            void fade_out_masking(uint8_t fadeLevel) {
                if (fadeLevel < 1 || fadeLevel > 5) return;
                const uint8_t patterns[5][2][2] = {
                    {{0, 0}, {0, 0}}, {{1, 0}, {0, 0}}, {{1, 0}, {0, 1}},
                    {{1, 0}, {1, 1}}, {{1, 1}, {1, 1}}
                };
                for (int16_t y = 0; y < 64; y++) {
                    int page = y / 8;
                    uint8_t pixel_mask = 1 << (y % 8);
                    for (int16_t x = 0; x < 128; x++) {
                        if (patterns[fadeLevel - 1][y % 2][x % 2]) {
                            buf_ptr[page * 128 + x] &= ~pixel_mask;
                        }
                    }
                }
            }
            uint8_t currentFadeLevel = (disappear_step - 1) / 3 + 1;
            fade_out_masking(currentFadeLevel);
            break;
        }

        case ANIM_SNOW_DISSOLVE: {
            animation_end_step = snow_steps;
            if (disappear_step > animation_end_step) break;

            const int pixels_per_step = 850; // Increased from 450
            if (disappear_step < snow_steps) {
                for (int i = 0; i < pixels_per_step; i++) {
                    int rand_x = rand() % 128;
                    int rand_y = rand() % 64;
                    int page = rand_y / 8;
                    uint8_t pixel_mask = 1 << (rand_y % 8);
                    buf_ptr[page * 128 + rand_x] &= ~pixel_mask;
                }
            } else { // On the last step, ensure it's fully clear
                u8g2_ClearBuffer(&u8g2);
            }
            break;
        }

        case ANIM_SLIDE_FROM_RIGHT: {
            animation_end_step = slide_steps;
            if (disappear_step > animation_end_step) break;
            uint16_t width = (128 * disappear_step) / slide_steps;
            uint16_t x = 128 - width;
            u8g2_SetClipWindow(&u8g2, x, 0, width, 64);
            draw_ui_by_index(ui_index);
            u8g2_SetMaxClipWindow(&u8g2);
            break;
        }

        case ANIM_SLIDE_FROM_LEFT: {
            animation_end_step = slide_steps;
            if (disappear_step > animation_end_step) break;
            uint16_t width = (128 * disappear_step) / slide_steps;
            u8g2_SetClipWindow(&u8g2, 0, 0, width, 64);
            draw_ui_by_index(ui_index);
            u8g2_SetMaxClipWindow(&u8g2);
            break;
        }

        case ANIM_SLIDE_FROM_BOTTOM: {
            animation_end_step = slide_steps;
            if (disappear_step > animation_end_step) break;
            uint16_t height = (64 * disappear_step) / slide_steps;
            uint16_t y = 64 - height;
            u8g2_SetClipWindow(&u8g2, 0, y, 128, height);
            draw_ui_by_index(ui_index);
            u8g2_SetMaxClipWindow(&u8g2);
            break;
        }

        case ANIM_SLIDE_FROM_TOP: {
            animation_end_step = slide_steps;
            if (disappear_step > animation_end_step) break;
            uint16_t height = (64 * disappear_step) / slide_steps;
            u8g2_SetClipWindow(&u8g2, 0, 0, 128, height);
            draw_ui_by_index(ui_index);
            u8g2_SetMaxClipWindow(&u8g2);
            break;
        }
    }

    // Step and state management
    disappear_step++;
    if (disappear_step > animation_end_step) {
        ui_state = S_NONE;
        disappear_step = 1; // Reset for the next transition
    }
}

bool move_pid(float *value, float target, pid_controller_t *pid)
{
    // Stop if close enough to the target and velocity is low
    if (fabsf(target - *value) < 0.5f && fabsf(pid->previous_error) < 0.5f) {
        *value = target;
        pid->integral = 0;
        pid->previous_error = 0;
        return true;
    }

    float error = target - *value;
    pid->integral += error;
    
    // Clamp integral to prevent windup
    if (pid->integral > 50) pid->integral = 50;
    if (pid->integral < -50) pid->integral = -50;

    float derivative = error - pid->previous_error;
    
    float output = (pid->kp * error) + (pid->ki * pid->integral) + (pid->kd * derivative);
    
    *value += output;
    pid->previous_error = error;
    
    return false;
}

void animator_init(animated_value_t *anim, float value) {
    anim->current = value;
    anim->target = value;
}

void animator_start(animated_value_t *anim, float target, float start) {
    anim->current = start;
    anim->target = target;
}

bool animator_run(animated_value_t *anim, pid_controller_t *pid) {
    return move_pid(&anim->current, anim->target, pid);
}