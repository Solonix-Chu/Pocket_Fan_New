#pragma once
#include <mooncake.h>
#include "lvgl.h"
#include "view/homepage_view.h"

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
    HomepageView* _view = nullptr;
    int _current_page = 0;
    uint32_t _last_update_time = 0;
    int _fan_speed = 0;
    uint32_t _last_scroll_time = 0;
    uint32_t _led_last_change_time = 0;
    uint32_t _last_input_time = 0;
    bool _closing_to_screensaver = false;

    void _create_view();
    void _destroy_view();
};
