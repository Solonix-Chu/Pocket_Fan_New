#include "app.h"
// #include "apps/launcher/launcher.h"
#include "apps/menu/menu.h"
// #include "apps/fan_control/fan_control.h"
// #include "apps/app_startup_anim/app_startup_anim.h"
#include "apps/apps.h"
#include "app_button.h"
#include "assets/assets.h"
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
    int screensaver_id = -1;
    int menu_id = -1;
    int settings_id = -1;
    int emoji_id = -1;
    int about_id = -1;
    int enjoy_id = -1;
    int health_id = -1;
    int detail_id = -1;
}

using namespace mooncake;

class SystemApp {
private:
    bool _is_startup_anim_done = false;
    int _startup_anim_id = -1;
    bool _poweroff_armed = false;

public:
    void init(lv_display_t* disp) {
        ESP_LOGI(TAG, "SystemApp init");
        HAL::LvglLock();

        // Load Assets
        LoadStaticAssets();
        AssetPool::SetLocaleCode(HAL::GetSystemConfig().localeCode);

        // Setup HAL for smooth_ui_toolkit
        smooth_ui_toolkit::ui_hal::on_get_tick([]() {
            static bool initialized = false;
            static uint64_t last_real_ms = 0;
            static uint64_t scaled_ms = 0;
            static double scaled_frac = 0.0;

            const uint64_t now_real_ms = static_cast<uint64_t>(esp_timer_get_time() / 1000);
            if (!initialized) {
                initialized = true;
                last_real_ms = now_real_ms;
                return static_cast<uint32_t>(0);
            }

            uint64_t dt_real_ms = 0;
            if (now_real_ms >= last_real_ms) {
                dt_real_ms = now_real_ms - last_real_ms;
            }
            last_real_ms = now_real_ms;

            int pct = 100;
            if (HAL::Check()) {
                pct = HAL::GetSystemConfig().uiAnimSpeedPct;
            }
            if (pct < 50) pct = 50;
            if (pct > 200) pct = 200;

            const double scale = static_cast<double>(pct) / 100.0;
            const double scaled_delta = static_cast<double>(dt_real_ms) * scale + scaled_frac;
            const uint64_t scaled_delta_i = static_cast<uint64_t>(scaled_delta);
            scaled_frac = scaled_delta - static_cast<double>(scaled_delta_i);
            scaled_ms += scaled_delta_i;

            // ui_hal tick is uint32_t by design.
            return static_cast<uint32_t>(scaled_ms);
        });

        // Install and Open Startup Anim
        _startup_anim_id = GetMooncake().installApp(std::make_unique<AppStartupAnim>());
        APPS::homepage_id = GetMooncake().installApp(std::make_unique<HomepageApp>());
        APPS::screensaver_id = GetMooncake().installApp(std::make_unique<ScreenSaverApp>());
        APPS::menu_id = GetMooncake().installApp(std::make_unique<MenuApp>());
        APPS::settings_id = GetMooncake().installApp(std::make_unique<SettingsApp>());
        APPS::emoji_id = GetMooncake().installApp(std::make_unique<EmojiApp>());
        APPS::about_id = GetMooncake().installApp(std::make_unique<AboutApp>());
        APPS::enjoy_id = GetMooncake().installApp(std::make_unique<EnjoyApp>());
        APPS::health_id = GetMooncake().installApp(std::make_unique<HealthApp>());
        APPS::detail_id = GetMooncake().installApp(std::make_unique<DetailApp>());

        GetMooncake().openApp(_startup_anim_id);
        HAL::LvglUnlock();
    }

    void update() {
        HAL::LvglLock();
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
        HAL::LvglUnlock();

        // Arm in-app power-off only after the first release, so the boot long-press
        // (>=2s) won't accidentally trigger a shutdown if the user keeps holding.
        if (!_poweroff_armed) {
            if (!HAL::IsPowerKeyPressed()) {
                _poweroff_armed = true;
            }
        } else if (HAL::GetButton(BUTTON::BTN_POWER) == APP_BUTTON_STATE_HOLD) {
            ESP_LOGI(TAG, "Power Button Long Press detected. Shutting down.");
            HAL::PowerOff();
        }

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
