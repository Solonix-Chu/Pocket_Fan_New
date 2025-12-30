/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen(lv_ui *ui)
{
    //Write codes screen
    ui->screen = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen, 128, 128);
    lv_obj_set_scrollbar_mode(ui->screen, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_start_logo_1
    ui->screen_start_logo_1 = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_start_logo_1, 1, 6);
    lv_obj_set_size(ui->screen_start_logo_1, 124, 22);
    lv_obj_add_flag(ui->screen_start_logo_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_start_logo_1, &_Start_Logo_RGB565A8_124x22);
    lv_image_set_pivot(ui->screen_start_logo_1, 50,50);
    lv_image_set_rotation(ui->screen_start_logo_1, 0);

    //Write style for screen_start_logo_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_start_logo_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_start_logo_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_about_icon
    ui->screen_about_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_about_icon, 4, 36);
    lv_obj_set_size(ui->screen_about_icon, 34, 26);
    lv_obj_add_flag(ui->screen_about_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_about_icon, &_About_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_about_icon, 50,50);
    lv_image_set_rotation(ui->screen_about_icon, 0);

    //Write style for screen_about_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_about_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_about_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_detail_icon
    ui->screen_detail_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_detail_icon, 46, 68);
    lv_obj_set_size(ui->screen_detail_icon, 34, 26);
    lv_obj_add_flag(ui->screen_detail_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_detail_icon, &_Detail_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_detail_icon, 50,50);
    lv_image_set_rotation(ui->screen_detail_icon, 0);

    //Write style for screen_detail_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_detail_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_detail_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_emoji_icon
    ui->screen_emoji_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_emoji_icon, 89, 39);
    lv_obj_set_size(ui->screen_emoji_icon, 34, 26);
    lv_obj_add_flag(ui->screen_emoji_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_emoji_icon, &_Emoji_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_emoji_icon, 50,50);
    lv_image_set_rotation(ui->screen_emoji_icon, 0);

    //Write style for screen_emoji_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_emoji_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_emoji_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_enjoy_icon
    ui->screen_enjoy_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_enjoy_icon, 7, 72);
    lv_obj_set_size(ui->screen_enjoy_icon, 34, 26);
    lv_obj_add_flag(ui->screen_enjoy_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_enjoy_icon, &_Enjoy_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_enjoy_icon, 50,50);
    lv_image_set_rotation(ui->screen_enjoy_icon, 0);

    //Write style for screen_enjoy_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_enjoy_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_enjoy_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_health_icon
    ui->screen_health_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_health_icon, 50, 36);
    lv_obj_set_size(ui->screen_health_icon, 34, 26);
    lv_obj_add_flag(ui->screen_health_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_health_icon, &_Health_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_health_icon, 50,50);
    lv_image_set_rotation(ui->screen_health_icon, 0);

    //Write style for screen_health_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_health_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_health_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_quit_icon
    ui->screen_quit_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_quit_icon, 89, 68);
    lv_obj_set_size(ui->screen_quit_icon, 34, 26);
    lv_obj_add_flag(ui->screen_quit_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_quit_icon, &_Quit_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_quit_icon, 50,50);
    lv_image_set_rotation(ui->screen_quit_icon, 0);

    //Write style for screen_quit_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_quit_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_quit_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_setting_icon
    ui->screen_setting_icon = lv_image_create(ui->screen);
    lv_obj_set_pos(ui->screen_setting_icon, 80, 98);
    lv_obj_set_size(ui->screen_setting_icon, 34, 26);
    lv_obj_add_flag(ui->screen_setting_icon, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_setting_icon, &_Setting_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_setting_icon, 50,50);
    lv_image_set_rotation(ui->screen_setting_icon, 0);

    //Write style for screen_setting_icon, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_setting_icon, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_setting_icon, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen);

}
