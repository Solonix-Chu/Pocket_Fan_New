#include "app_startup_anim.h"
#include <esp_timer.h>
#include "../../assets/assets.h"

StartupAnimApp::StartupAnimApp() {
    // We can set ID manually if needed, but Mooncake assigns it on install.
    setAppInfo().name = "StartupAnim";
}

StartupAnimApp::~StartupAnimApp() {
    onDestroy();
}

void StartupAnimApp::onOpen() {
    if (!_screen) {
        _screen = lv_obj_create(NULL);
        lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
        
        lv_obj_t* img = lv_image_create(_screen);
        
        static lv_image_dsc_t img_dsc;
        img_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
        img_dsc.header.cf = LV_COLOR_FORMAT_RGB565A8;
        img_dsc.header.flags = 0;
        img_dsc.header.w = 124;
        img_dsc.header.h = 22;
        img_dsc.header.stride = 248;
        img_dsc.data_size = 8184;
        img_dsc.data = AssetPool::GetImage().Startup.logo;

        lv_image_set_src(img, &img_dsc);
        lv_obj_center(img);
    }
    lv_scr_load(_screen);
    _start_time = esp_timer_get_time() / 1000;
}

void StartupAnimApp::onRunning() {
    // Show for 2 seconds (2000 ms)
    if ((esp_timer_get_time() / 1000) - _start_time > 1000) {
        // Close self
        mooncake::GetMooncake().closeApp(getID());
        
        // Open Launcher (Assuming ID 0 is Launcher)
        // mooncake::GetMooncake().openApp(0);
    }
}

void StartupAnimApp::onClose() {
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void StartupAnimApp::onDestroy() {
    onClose();
}