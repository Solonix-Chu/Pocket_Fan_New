#include "screensaver.h"
#include "../../hal/hal.h"
#include "../apps.h"
#include "app_button.h"
#include "space_man_bitmaps.h"
#include <esp_log.h>
#include <cstring>

static const char* TAG = "ScreenSaverApp";

namespace {
static constexpr uint32_t kFrameW = 128;
static constexpr uint32_t kFrameH = 64;
static constexpr uint32_t kFrameBytes = (kFrameW * kFrameH) / 8; // 1024
static constexpr uint32_t kPaletteBytes = 8;                     // 2 * lv_color32_t
static constexpr uint32_t kFrameIntervalMs = 40;                 // ~25fps (matches demo)

static constexpr uint32_t kFrameCount = 20;

static const unsigned char* const kFrames[kFrameCount] = {
    pocket_fan::screensaver::gImage_1,
    pocket_fan::screensaver::gImage_2,
    pocket_fan::screensaver::gImage_3,
    pocket_fan::screensaver::gImage_4,
    pocket_fan::screensaver::gImage_5,
    pocket_fan::screensaver::gImage_6,
    pocket_fan::screensaver::gImage_7,
    pocket_fan::screensaver::gImage_8,
    pocket_fan::screensaver::gImage_9,
    pocket_fan::screensaver::gImage_10,
    pocket_fan::screensaver::gImage_11,
    pocket_fan::screensaver::gImage_12,
    pocket_fan::screensaver::gImage_13,
    pocket_fan::screensaver::gImage_14,
    pocket_fan::screensaver::gImage_15,
    pocket_fan::screensaver::gImage_16,
    pocket_fan::screensaver::gImage_17,
    pocket_fan::screensaver::gImage_18,
    pocket_fan::screensaver::gImage_19,
    pocket_fan::screensaver::gImage_20,
};
} // namespace

ScreenSaverApp::ScreenSaverApp() {
    setAppInfo().name = "ScreenSaver";
    setAppInfo().icon = nullptr;
}

void ScreenSaverApp::onCreate() {
    // App info already set in ctor for convenience.
}

void ScreenSaverApp::onOpen() {
    ESP_LOGI(TAG, "onOpen");

    _screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_screen, 0, 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);

    _img = lv_image_create(_screen);
    lv_obj_align(_img, LV_ALIGN_CENTER, 0, 0);

    // LV_COLOR_FORMAT_I1 palette (lv_color32_t = {B,G,R,A})
    // index 0 -> background (white), index 1 -> foreground (black)
    _frame_buf[0] = 0xFF;
    _frame_buf[1] = 0xFF;
    _frame_buf[2] = 0xFF;
    _frame_buf[3] = 0xFF;
    _frame_buf[4] = 0x00;
    _frame_buf[5] = 0x00;
    _frame_buf[6] = 0x00;
    _frame_buf[7] = 0xFF;

    _frame_dsc.header.cf = LV_COLOR_FORMAT_I1;
    _frame_dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    _frame_dsc.header.w = kFrameW;
    _frame_dsc.header.h = kFrameH;
    _frame_dsc.header.stride = kFrameW / 8;
    _frame_dsc.data_size = kPaletteBytes + kFrameBytes;
    _frame_dsc.data = _frame_buf.data();

    _frame_idx = 0;
    _last_frame_ms = HAL::Millis();
    _renderFrame(_frame_idx);

    lv_image_set_src(_img, &_frame_dsc);
    lv_scr_load(_screen);
    lv_obj_invalidate(_screen);
}

bool ScreenSaverApp::_hasAnyInput() const {
    return (HAL::GetButton(BUTTON::BTN_MID) != APP_BUTTON_STATE_NOCHANGE) ||
           (HAL::GetButton(BUTTON::BTN_LEFT) != APP_BUTTON_STATE_NOCHANGE) ||
           (HAL::GetButton(BUTTON::BTN_RIGHT) != APP_BUTTON_STATE_NOCHANGE) ||
           (HAL::GetButton(BUTTON::BTN_UP) != APP_BUTTON_STATE_NOCHANGE) ||
           (HAL::GetButton(BUTTON::BTN_DOWN) != APP_BUTTON_STATE_NOCHANGE) ||
           (HAL::GetButton(BUTTON::BTN_POWER) != APP_BUTTON_STATE_NOCHANGE);
}

void ScreenSaverApp::_consumeAllInput() {
    if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
    if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
    if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
    if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
    if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
    if (BtnPower) BtnPower->currentState = APP_BUTTON_STATE_NOCHANGE;
}

void ScreenSaverApp::_renderFrame(uint32_t frame_idx) {
    if (frame_idx >= kFrameCount) {
        frame_idx = 0;
    }

    const unsigned char* frame = kFrames[frame_idx];
    if (!frame) {
        return;
    }

    std::memcpy(_frame_buf.data() + kPaletteBytes, frame, kFrameBytes);
    if (_img) {
        lv_obj_invalidate(_img);
    }
}

void ScreenSaverApp::onRunning() {
    if (_hasAnyInput()) {
        _consumeAllInput();
        mooncake::GetMooncake().openApp(APPS::homepage_id);
        close();
        return;
    }

    uint32_t now = HAL::Millis();
    if (now - _last_frame_ms >= kFrameIntervalMs) {
        _last_frame_ms = now;
        _frame_idx = (_frame_idx + 1) % kFrameCount;
        _renderFrame(_frame_idx);
    }
}

void ScreenSaverApp::onClose() {
    ESP_LOGI(TAG, "onClose");
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        _img = nullptr;
    }
}

void ScreenSaverApp::onDestroy() {
    onClose();
}
