#pragma once
#include <mooncake.h>

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
    int _current_page = 0;

    void _create_view();
    void _destroy_view();
};
