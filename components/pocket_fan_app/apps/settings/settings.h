#pragma once
#include <mooncake.h>
#include "view/settings_view.h"

class SettingsApp : public mooncake::AppAbility
{
public:
    SettingsApp() = default;
    ~SettingsApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    SettingsView* _view = nullptr;
    void _create_view();
    void _destroy_view();
};
