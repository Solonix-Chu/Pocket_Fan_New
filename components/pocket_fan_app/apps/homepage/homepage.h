#pragma once
#include <mooncake.h>
#include "lvgl.h"

class HomepageApp : public mooncake::AppAbility
{
public:
    HomepageApp() = default;
    ~HomepageApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _tileview = nullptr;
    lv_obj_t* _tile1 = nullptr;
    lv_obj_t* _tile2 = nullptr;
    lv_obj_t* _label_v = nullptr;
    lv_obj_t* _label_a = nullptr;
    lv_obj_t* _label_w = nullptr;
    int _current_page = 0;
    uint32_t _last_update_time = 0;

    void _create_view();
    void _destroy_view();
};
