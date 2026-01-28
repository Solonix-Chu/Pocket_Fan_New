#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>
#include "../../../ui/transitions.h"

using namespace smooth_ui_toolkit;

class HomepageView : public SmoothSelectorMenu {
public:
    HomepageView();
    ~HomepageView();

    void init();
    
    // Telemetry updates
    void updateData(const std::string& v, const std::string& a, const std::string& w);
    void updatePage2Data(const std::string& t, const std::string& cap);
    void updateIp2369(const std::string& title,
                      const std::string& in,
                      const std::string& out,
                      const std::string& power,
                      const std::string& ntc);
    void updatePwm(int dutyCycle);
    
    // Page switching
    void setPage(int page);
    void tick(uint32_t now_ms);
    void restartEntry();

    // Overrides
    void onRender() override;
    void onReadInput() override;
    void onClick() override;

private:
    // LVGL Objects
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _tileview = nullptr;
    lv_obj_t* _tile1 = nullptr;
    lv_obj_t* _tile2 = nullptr;
    lv_obj_t* _tile3 = nullptr;
    lv_obj_t* _bg1 = nullptr;
    lv_obj_t* _bg2 = nullptr;
    lv_obj_t* _bg3 = nullptr;
    
    lv_obj_t* _label_v = nullptr;
    lv_obj_t* _label_a = nullptr;
    lv_obj_t* _label_w = nullptr;
    lv_obj_t* _label_t = nullptr;
    lv_obj_t* _label_cap = nullptr;
    lv_obj_t* _label_ip_title = nullptr;
    lv_obj_t* _label_ip_in = nullptr;
    lv_obj_t* _label_ip_out = nullptr;
    lv_obj_t* _label_ip_power = nullptr;
    lv_obj_t* _label_ip_ntc = nullptr;
    lv_obj_t* _label_pwm = nullptr;

    void _create_lvgl_objects();
    pocket_fan::ui::TransitionValue _entry_y;
    bool _entry_started = false;
};
