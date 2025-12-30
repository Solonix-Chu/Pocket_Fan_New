#include "app.h"
#include "../../hal/hal.h"
#include <va_effects.h>
#include <LovyanGFX.hpp>

AnimationTestApp::AnimationTestApp()
{
}

static void btn_event_cb(lv_event_t * e)
{
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    const char * label = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (strcmp(label, "Fade Out") == 0) {
        VaEffect::FadeBacklight(20, 1000);
    } else if (strcmp(label, "Fade In") == 0) {
        VaEffect::FadeBacklight(200, 1000);
    } else if (strcmp(label, "Circle Reveal") == 0) {
        VaEffect::CircularReveal(0, 0, 64, 32, 100, 0x00FF00, 1000);
    } else if (strcmp(label, "Slide In") == 0) {
        VaEffect::SlideTransition(VaEffect::Direction::Up, 64, true, [](int x, int y) {
            HAL::GetCanvas()->fillRect(x, y, 128, 64, 0x0000FF);
            HAL::GetCanvas()->setTextColor(0xFFFFFF);
            HAL::GetCanvas()->drawString("Sliding In...", x + 20, y + 25);
        }, 1000);
    }
}

void AnimationTestApp::onOpen()
{
    // Initialize VaEffect with HAL
    VaEffect::Init(HAL::GetDisplay(), (LGFX_Sprite*)HAL::GetCanvas(), [](){ HAL::FeedTheDog(); });

    // Initialize screen
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_scr_load(_screen);

    // List for gallery
    lv_obj_t * list = lv_list_create(_screen);
    lv_obj_set_size(list, 128, 64);
    lv_obj_center(list);

    const char * btn_labels[] = {"Fade Out", "Fade In", "Circle Reveal", "Slide In"};
    for (const char * lbl : btn_labels) {
        lv_obj_t * btn = lv_list_add_btn(list, NULL, lbl);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);
    }
}

void AnimationTestApp::onRunning()
{
    // Check for back button (hardware)
    if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_HOLD) {
        close();
    }
}

void AnimationTestApp::onClose()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void AnimationTestApp::onDestroy()
{
    // Cleanup
}