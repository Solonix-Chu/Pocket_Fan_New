#pragma once
#include <mooncake.h>
#include <string>
#include "../settings/view/settings_view.h"

class DetailApp : public mooncake::AppAbility {
public:
    DetailApp() = default;
    ~DetailApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _render();
    std::string _label(int index) const;
    std::string _formatVoltage(float voltage) const;
    std::string _formatCurrent(float current) const;
    std::string _formatPower(float power) const;
    std::string _formatTemp(float celsius) const;
    float _calcBatteryPercent(float vbat) const;

    SettingsView* _view = nullptr;
};
