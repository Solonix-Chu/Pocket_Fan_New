#pragma once
#include <mooncake.h>
#include <lvgl.h>

class EnjoyApp : public mooncake::AppAbility {
public:
    EnjoyApp();
    ~EnjoyApp() override = default;

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _applyInput(float dt);
    void _stepPhysics(float dt);
    void _updateBall();
    void _bounceColor();

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _ball = nullptr;
    lv_obj_t* _hint = nullptr;

    float _x = 64.0f;
    float _y = 32.0f;
    float _vx = 0.0f;
    float _vy = 0.0f;
    float _radius = 6.0f;
    float _screen_w = 128.0f;
    float _screen_h = 64.0f;
    uint32_t _last_ms = 0;
};
