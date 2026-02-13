#pragma once
#include <mooncake.h>
#include <lvgl.h>
#include "../settings/view/settings_view.h"

class HealthApp : public mooncake::AppAbility {
public:
    HealthApp();
    ~HealthApp() override = default;

    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _updateStats(float dt_hours);
    void _render();
    float _calcBatteryHealth() const;
    float _calcMotorHealth() const;

    SettingsView* _view = nullptr;

    uint32_t _last_ms = 0;
};
