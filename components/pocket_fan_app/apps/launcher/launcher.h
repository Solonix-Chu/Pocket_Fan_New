#pragma once
#include <mooncake.h>
#include <mooncake_templates.h>
#include <smooth_ui_toolkit.h>
#include <smooth_lvgl.h>
#include <lvgl.h>
#include <vector>
#include <string>

using namespace mooncake;
using namespace smooth_ui_toolkit;

class LauncherApp : public templates::AppLauncherBase, public SmoothSelectorMenu {
public:
    void onLauncherCreate() override;
    void onLauncherOpen() override;
    void onLauncherRunning() override;
    void onLauncherSleeping() override;
    void onLauncherClose() override;
    void onLauncherDestroy() override;

    // SmoothSelectorMenu overrides
    void onRender() override;
    void onReadInput() override;
    void onClick() override;

private:
    lv_obj_t* _screen = nullptr;
    lv_obj_t* _selector_obj = nullptr;
    std::vector<lv_obj_t*> _option_labels;
    std::vector<std::string> _option_names;
    std::vector<int> _app_ids;

    void _create_ui();
    void _destroy_ui();
    void _update_ui();
};
