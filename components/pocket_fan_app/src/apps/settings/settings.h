#pragma once
#include <mooncake.h>
#include <smooth_ui_toolkit.h>
#include <lvgl.h>
#include <vector>
#include <string>

using namespace mooncake;
using namespace smooth_ui_toolkit;

class SettingsApp : public AppAbility, public SmoothSelectorMenu {
public:
    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

    // SmoothSelectorMenu overrides
    void onRender() override;
    void onReadInput() override;
    void onClick() override;

private:
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    std::vector<lv_obj_t*> _option_labels;
    std::vector<std::string> _settings_items = {"Brightness", "Sound", "About", "Back"};

    void _create_ui();
    void _destroy_ui();
};
