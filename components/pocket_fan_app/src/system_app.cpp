#include "system_app.h"
#include "apps/launcher/launcher.h"
#include "apps/settings/settings.h"
#include "apps/fan_control/fan_control.h"
#include "apps/app_startup_anim/app_startup_anim.h"
#include "app_button.h"
#include <mooncake.h>
#include <smooth_ui_toolkit.h>
#include <esp_timer.h>
#include <memory>
#include "esp_log.h"
static const char *TAG = "PocketFan_App";

using namespace mooncake;

class SystemApp {
public:
    void init(lv_display_t* disp) {
        ESP_LOGI(TAG, "SystemApp init");

        // Setup HAL for smooth_ui_toolkit
        smooth_ui_toolkit::ui_hal::on_get_tick([]() {
            return (uint32_t)(esp_timer_get_time() / 1000);
        });

        // Install Apps
        // ID 0: Launcher
        GetMooncake().installApp(std::make_unique<LauncherApp>());
        
        // ID 1: Startup Anim
        int startup_id = GetMooncake().installApp(std::make_unique<StartupAnimApp>());
        
        // ID 2: Fan Control
        // GetMooncake().installApp(std::make_unique<FanControlApp>());
        
        // // ID 3: Settings
        // GetMooncake().installApp(std::make_unique<SettingsApp>());

        // Open Startup Anim
        GetMooncake().openApp(startup_id);
    }

    void update() {
        GetMooncake().update();
        app_button_update();
    }
};

static SystemApp app;

extern "C" void system_app_init(lv_display_t* disp) {
    app.init(disp);
}

extern "C" void system_app_update(void) {
    app.update();
}