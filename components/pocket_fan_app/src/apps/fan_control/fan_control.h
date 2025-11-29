#pragma once
#include <mooncake.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <lvgl.h>

using namespace mooncake;
using namespace smooth_ui_toolkit;
using namespace smooth_ui_toolkit::lvgl_cpp;

class FanControlApp : public AppAbility {
public:
    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    lv_obj_t* _screen = nullptr;
    NumberFlow* _number_flow = nullptr;
    int _speed = 50;

    void _create_ui();
    void _destroy_ui();
    void _update_input();
};
