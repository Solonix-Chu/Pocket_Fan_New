#include "app.h"
#include <LovyanGFX.hpp>

AnimationTestApp::AnimationTestApp()
{
    setAppInfo().name = "AnimationTestApp";
}

void AnimationTestApp::onOpen()
{
    // Initialize screen
    _screen = lv_obj_create(NULL);
    lv_scr_load(_screen);

    // Add a simple label
    lv_obj_t *label = lv_label_create(_screen);
    lv_label_set_text(label, "Animation Test App");
    lv_obj_center(label);

    // Simple back button usage instruction
    lv_obj_t *hint = lv_label_create(_screen);
    lv_label_set_text(hint, "Press Back to Exit");
    lv_obj_align(hint, LV_ALIGN_BOTTOM_MID, 0, -20);
}

void AnimationTestApp::onRunning()
{
    // Update logic here
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