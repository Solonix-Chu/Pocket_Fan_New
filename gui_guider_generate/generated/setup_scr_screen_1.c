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



void setup_scr_screen_1(lv_ui *ui)
{
    //Write codes screen_1
    ui->screen_1 = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_1, 128, 128);
    lv_obj_set_scrollbar_mode(ui->screen_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_1
    ui->screen_1_img_1 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_1, 9, 13);
    lv_obj_set_size(ui->screen_1_img_1, 34, 25);
    lv_obj_add_flag(ui->screen_1_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_1, &_1_RGB565A8_34x25);
    lv_image_set_pivot(ui->screen_1_img_1, 50,50);
    lv_image_set_rotation(ui->screen_1_img_1, 0);

    //Write style for screen_1_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_2
    ui->screen_1_img_2 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_2, 48, 13);
    lv_obj_set_size(ui->screen_1_img_2, 34, 26);
    lv_obj_add_flag(ui->screen_1_img_2, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_2, &_2_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_1_img_2, 50,50);
    lv_image_set_rotation(ui->screen_1_img_2, 0);

    //Write style for screen_1_img_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_3
    ui->screen_1_img_3 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_3, 88, 13);
    lv_obj_set_size(ui->screen_1_img_3, 34, 26);
    lv_obj_add_flag(ui->screen_1_img_3, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_3, &_3_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_1_img_3, 50,50);
    lv_image_set_rotation(ui->screen_1_img_3, 0);

    //Write style for screen_1_img_3, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_3, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_3, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_4
    ui->screen_1_img_4 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_4, 9, 48);
    lv_obj_set_size(ui->screen_1_img_4, 34, 26);
    lv_obj_add_flag(ui->screen_1_img_4, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_4, &_4_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_1_img_4, 50,50);
    lv_image_set_rotation(ui->screen_1_img_4, 0);

    //Write style for screen_1_img_4, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_4, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_4, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_5
    ui->screen_1_img_5 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_5, 52, 48);
    lv_obj_set_size(ui->screen_1_img_5, 34, 26);
    lv_obj_add_flag(ui->screen_1_img_5, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_5, &_5_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_1_img_5, 50,50);
    lv_image_set_rotation(ui->screen_1_img_5, 0);

    //Write style for screen_1_img_5, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_5, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_5, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_1_img_6
    ui->screen_1_img_6 = lv_image_create(ui->screen_1);
    lv_obj_set_pos(ui->screen_1_img_6, 88, 48);
    lv_obj_set_size(ui->screen_1_img_6, 34, 26);
    lv_obj_add_flag(ui->screen_1_img_6, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_1_img_6, &_6_RGB565A8_34x26);
    lv_image_set_pivot(ui->screen_1_img_6, 50,50);
    lv_image_set_rotation(ui->screen_1_img_6, 0);

    //Write style for screen_1_img_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_1_img_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_1_img_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of screen_1.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_1);

}
