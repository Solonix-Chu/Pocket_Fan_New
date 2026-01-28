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
    bool _is_adjusting_brightness = false;
    int _brightness_val = 255;
    bool _invert_display = false;
    bool _is_black_theme = false;
    int _theme_item_index = -1;
    int _invert_item_index = -1;
    uint32_t _last_scroll_time = 0;
    
    void _create_view();
    void _destroy_view();
};
