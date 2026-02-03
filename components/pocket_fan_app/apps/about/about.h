#pragma once
#include <mooncake.h>
#include <lvgl.h>

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
};

