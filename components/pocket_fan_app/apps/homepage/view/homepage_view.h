#pragma once
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include <string>
#include <functional>

using namespace smooth_ui_toolkit;

class HomepageView : public SmoothSelectorMenu {
public:
    HomepageView();
    ~HomepageView();

    void init();
    
    // Telemetry updates
    void updateData(const std::string& v, const std::string& a, const std::string& w);
    void updatePage2Data(const std::string& t, const std::string& cap);
    
    // Page switching
    void setPage(int page);

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
    lv_obj_t* _bg1 = nullptr;
    lv_obj_t* _bg2 = nullptr;
    
    lv_obj_t* _label_v = nullptr;
    lv_obj_t* _label_a = nullptr;
    lv_obj_t* _label_w = nullptr;
    lv_obj_t* _label_t = nullptr;
    lv_obj_t* _label_cap = nullptr;

    void _create_lvgl_objects();
};
