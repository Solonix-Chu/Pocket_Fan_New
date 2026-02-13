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
    enum class PopupMode : uint8_t {
        None = 0,
        Brightness,
        AnimSpeed,
        Trackball,
        FactoryResetConfirm,
        EscCalibrating,
    };

    SettingsView* _view = nullptr;
    PopupMode _popup_mode = PopupMode::None;

    // Brightness popup
    int _brightness_val = 255;
    int _brightness_before = 255;

    // Theme (display inversion) checkbox
    bool _theme_black = false;
    int _theme_item_index = -1;

    // Animation speed popup
    int _ui_anim_speed_pct = 100;
    int _ui_anim_speed_before = 100;

    // Trackball popup state (mode + RGBW)
    int _trackball_field = 0; // 0=mode, 1=R, 2=G, 3=B, 4=W
    int _trackball_mode = 0;
    int _trackball_mode_before = 0;
    int _trackball_r = 0;
    int _trackball_g = 0;
    int _trackball_b = 0;
    int _trackball_w = 0;
    int _trackball_r_before = 0;
    int _trackball_g_before = 0;
    int _trackball_b_before = 0;
    int _trackball_w_before = 0;

    int _language_item_index = -1;
    uint32_t _last_scroll_time = 0;
    bool _request_rebuild_view = false;
    int _rebuild_selected_index = 0;
    
    void _create_view(int initial_index = 0, bool skip_entry_anim = false);
    void _destroy_view();

    void _enter_popup(PopupMode mode);
    void _exit_popup(bool apply_changes);
    void _render_trackball_popup();
};
