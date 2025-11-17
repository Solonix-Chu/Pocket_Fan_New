#include "include/smooth_ui_demo.h"
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>
#include "esp_timer.h"

using namespace smooth_ui_toolkit;

// Create a custom menu class
class MyMenu : public SmoothSelectorMenu
{
private:
    lv_obj_t* _screen;
    lv_obj_t* _selector_obj;

public:
    MyMenu(lv_obj_t* screen)
    {
        _screen = screen;

        // Create a canvas for the selector
        _selector_obj = lv_obj_create(_screen);
        lv_obj_set_size(_selector_obj, 10, 10);
        lv_obj_set_style_bg_color(_selector_obj, lv_color_hex(0xff0000), 0);
        lv_obj_set_style_radius(_selector_obj, LV_RADIUS_CIRCLE, 0);
        lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);
    }

    void onRender() override
    {
        // Update selector position and size
        auto selector_frame = getSelectorCurrentFrame();
        lv_obj_set_pos(_selector_obj, selector_frame.x, selector_frame.y);
        lv_obj_set_size(_selector_obj, selector_frame.width, selector_frame.height);
    }
};

static MyMenu* my_menu;

static void update_timer_cb(lv_timer_t * timer)
{
    if (my_menu) {
        my_menu->update();

        // Automatically go to next option every second
        static uint32_t last_tick = 0;
        if (lv_tick_get() - last_tick > 1000) {
            last_tick = lv_tick_get();
            my_menu->goNext();
        }
    }
}

void smooth_ui_demo(lv_display_t *disp)
{
    // Set HAL tick function
    smooth_ui_toolkit::ui_hal::on_get_tick([]() {
        return (uint32_t)(esp_timer_get_time() / 1000);
    });

    lv_obj_t *scr = lv_display_get_screen_active(disp);

    // Create a menu instance
    my_menu = new MyMenu(scr);

    // Config
    auto& cfg = my_menu->setConfig();
    cfg.renderInterval = 10;  // 这个参数设置了渲染的间隔时间（单位：毫秒）。它决定了动效更新的频率，值越小，动画看起来就越平滑，但也会占用更多的 CPU 时间。
    cfg.cameraSize = { (float)lv_disp_get_hor_res(disp), (float)lv_disp_get_ver_res(disp) };

    // Add options
    my_menu->addOption({{10, 10, 20, 20}});
    my_menu->addOption({{60, 10, 20, 20}});
    my_menu->addOption({{110, 10, 20, 20}});
    my_menu->addOption({{110, 100, 20, 20}});
    my_menu->addOption({{60, 50, 20, 20}});
    my_menu->addOption({{10, 200, 20, 20}});

    // Start at the first option
    my_menu->jumpTo(0);

    // Create a timer to update the menu
    lv_timer_create(update_timer_cb, 10, nullptr);
}
