#pragma once
#include <mooncake.h>
#include <lvgl.h>

class AnimationTestApp : public mooncake::AppAbility {
public:
    AnimationTestApp();
    ~AnimationTestApp() override = default;

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

    void startStaggerListDemo();
    void startPanelSlideDemo();
    void startMaskRevealDemo();

private:
    lv_obj_t* _screen = nullptr;
};
