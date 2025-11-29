#include "app_startup_anim.h"
#include <esp_timer.h>

// Declare the image asset
extern const lv_image_dsc_t _start_124X22_RGB565A8_124x22;

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
        lv_image_set_src(img, &_start_124X22_RGB565A8_124x22);
        lv_obj_center(img);
    }
    lv_scr_load(_screen);
    _start_time = esp_timer_get_time() / 1000;
}

void StartupAnimApp::onRunning() {
    // Show for 2 seconds (2000 ms)
    if ((esp_timer_get_time() / 1000) - _start_time > 2000) {
        // Close self
        mooncake::GetMooncake().closeApp(getID());
        
        // Open Launcher (Assuming ID 0 is Launcher)
        mooncake::GetMooncake().openApp(0);
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