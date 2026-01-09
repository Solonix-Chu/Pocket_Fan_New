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
    lv_obj_t* _mask = nullptr;
    uint32_t _start_time = 0;
    uint32_t _finish_time = 0;
    pocket_fan::ui::TransitionValue _backlight;
    pocket_fan::ui::TransitionValue _mask_translate;

    static bool _guide_active;
};
