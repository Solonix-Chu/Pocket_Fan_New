#pragma once
#include <mooncake.h>
#include <lvgl.h>

class StartupAnimApp : public mooncake::AppAbility {
public:
    StartupAnimApp();
    ~StartupAnimApp() override;

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    lv_obj_t* _screen = nullptr;
    uint32_t _start_time = 0;
};