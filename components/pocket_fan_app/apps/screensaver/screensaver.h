#pragma once
#include <mooncake.h>
#include <lvgl.h>
#include <array>
#include <cstdint>

class ScreenSaverApp : public mooncake::AppAbility {
public:
    ScreenSaverApp();
    ~ScreenSaverApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _renderFrame(uint32_t frame_idx);
    bool _hasAnyInput() const;
    void _consumeAllInput();

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _img = nullptr;

    lv_image_dsc_t _frame_dsc{};
    std::array<uint8_t, 8 + 1024> _frame_buf{};

    uint32_t _frame_idx = 0;
    uint32_t _last_frame_ms = 0;
};

