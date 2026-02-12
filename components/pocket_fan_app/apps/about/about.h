#pragma once
#include <mooncake.h>
#include <lvgl.h>
#include <cstdint>

class AboutApp : public mooncake::AppAbility
{
public:
    AboutApp() = default;
    ~AboutApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _create_view();
    void _destroy_view();
    void _update_labels();
    bool _can_exit() const;

    lv_obj_t* _screen = nullptr;
    lv_obj_t* _label_title = nullptr;
    lv_obj_t* _label_fw = nullptr;
    lv_obj_t* _label_name = nullptr;
    lv_obj_t* _label_mac = nullptr;
    lv_obj_t* _label_status = nullptr;
    lv_obj_t* _label_progress = nullptr;

    char _mac_str[32] = {0};

    // Cache the last values rendered to labels to avoid resetting LVGL marquee
    // animations (LV_LABEL_LONG_SCROLL_CIRCULAR) every frame.
    uint8_t _last_state = 0xFF;
    uint32_t _last_received_len = static_cast<uint32_t>(-1);
    uint32_t _last_total_len = static_cast<uint32_t>(-1);
    int32_t _last_init_error = static_cast<int32_t>(0x7fffffff);
    unsigned long _last_progress_update_ms = 0;
};
