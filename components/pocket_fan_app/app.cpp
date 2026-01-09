#include "app.h"
// #include "apps/launcher/launcher.h"
#include "apps/menu/menu.h"
// #include "apps/fan_control/fan_control.h"
// #include "apps/app_startup_anim/app_startup_anim.h"
#include "apps/apps.h"
#include "app_button.h"
#include "assets/assets_loader.h"
#include <mooncake.h>
#include <smooth_ui_toolkit.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <memory>
#include "esp_log.h"
static const char *TAG = "PocketFan_App";

namespace APPS {
    int homepage_id = -1;
    int menu_id = -1;
    int settings_id = -1;
    int animation_test_id = -1;
}

using namespace mooncake;

class SystemApp {
private:
    bool _is_startup_anim_done = false;
    int _startup_anim_id = -1;

public:
    void init(lv_display_t* disp) {
        ESP_LOGI(TAG, "SystemApp init");

        // Load Assets
        LoadStaticAssets();

        // Setup HAL for smooth_ui_toolkit
        smooth_ui_toolkit::ui_hal::on_get_tick([]() {
            return (uint32_t)(esp_timer_get_time() / 1000);
        });

        // Install and Open Startup Anim
        _startup_anim_id = GetMooncake().installApp(std::make_unique<AppStartupAnim>());
        APPS::homepage_id = GetMooncake().installApp(std::make_unique<HomepageApp>());
        APPS::menu_id = GetMooncake().installApp(std::make_unique<MenuApp>());
        APPS::settings_id = GetMooncake().installApp(std::make_unique<SettingsApp>());
        APPS::animation_test_id = GetMooncake().installApp(std::make_unique<AnimationTestApp>());

        GetMooncake().openApp(_startup_anim_id);
    }

    void update() {
        // Check startup anim status
        if (!_is_startup_anim_done) {
            // ESP_LOGI(TAG, "StartupAnimApp state: %d", (int)GetMooncake().getAppCurrentState(_startup_anim_id));
            if (GetMooncake().getAppCurrentState(_startup_anim_id) == mooncake::AppAbility::StateGoClose) {
                // Startup finished
                GetMooncake().uninstallApp(_startup_anim_id);

                // Open Animation Test App instead of Homepage for verification
                // GetMooncake().openApp(APPS::animation_test_id);
                GetMooncake().openApp(APPS::homepage_id);
                
                _is_startup_anim_done = true;
            }
        }

        GetMooncake().update();
        HAL::Get()->LGVL_UPDATE();

        // if (HAL::GetButton(BUTTON::BTN_POWER) == APP_BUTTON_STATE_HOLD) {
        //     ESP_LOGI(TAG, "Power Button Long Press detected. Shutting down.");
        //     HAL::PowerOff();
        // }

        HAL::Get()->allButton_refresh(); // 每个周期最后调用，刷新按键状态

        // Feed task watchdog to avoid false triggers during UI animations
        esp_task_wdt_reset();
    }
};

static SystemApp app;

extern "C" void app_init(lv_display_t* disp) {
    app.init(disp);
}

extern "C" void app_update(void) {
    app.update();
}
