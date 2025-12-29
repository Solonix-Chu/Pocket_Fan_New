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
#include <memory>
#include "esp_log.h"
static const char *TAG = "PocketFan_App";

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
        _startup_anim_id = GetMooncake().installApp(std::make_unique<StartupAnimApp>());
        GetMooncake().openApp(_startup_anim_id);
        // int test_id = GetMooncake().installApp(std::make_unique<HardwareTestApp>());
        //         GetMooncake().openApp(test_id);
    }

    void update() {
        // Check startup anim status
        if (!_is_startup_anim_done) {
            // ESP_LOGI(TAG, "StartupAnimApp state: %d", (int)GetMooncake().getAppCurrentState(_startup_anim_id));
            if (GetMooncake().getAppCurrentState(_startup_anim_id) == mooncake::AppAbility::StateGoClose) {
                // Startup finished
                GetMooncake().uninstallApp(_startup_anim_id);
                
                // Install Hardware Test App
                // int test_id = GetMooncake().installApp(std::make_unique<HardwareTestApp>());
                // 
                int test_id = GetMooncake().installApp(std::make_unique<HomepageApp>());
                GetMooncake().openApp(test_id);
                
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
    }
};

static SystemApp app;

extern "C" void app_init(lv_display_t* disp) {
    app.init(disp);
}

extern "C" void app_update(void) {
    app.update();
}