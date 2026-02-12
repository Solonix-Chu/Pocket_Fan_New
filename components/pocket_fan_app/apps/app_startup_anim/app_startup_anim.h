#pragma once
#include <mooncake.h>
#include <lvgl.h>
#include "../../ui/transitions.h"

class AppStartupAnim : public mooncake::AppAbility {
public:
    AppStartupAnim();
    ~AppStartupAnim() override;

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

    static void PopUpGuideMap(bool force = false);

private:
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _progress = nullptr;
    uint32_t _start_time = 0;

    static bool _guide_active;
};
