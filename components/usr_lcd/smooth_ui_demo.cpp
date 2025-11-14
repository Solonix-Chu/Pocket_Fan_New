#include "include/smooth_ui_demo.h"
#include "smooth_ui_toolkit.h"
#include "lvgl.h"
#include <vector>

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
    }
}

void smooth_ui_demo(lv_display_t *disp)
{
    lv_obj_t *scr = lv_display_get_screen_active(disp);

    // Create a menu instance
    my_menu = new MyMenu(scr);

    // Config
    auto& cfg = my_menu->setConfig();
    cfg.renderInterval = 10;
    cfg.cameraSize = { (float)lv_disp_get_hor_res(disp), (float)lv_disp_get_ver_res(disp) };

    // Add options
    my_menu->addOption({{10, 10, 20, 20}});
    my_menu->addOption({{60, 10, 20, 20}});
    my_menu->addOption({{110, 10, 20, 20}});
    my_menu->addOption({{110, 60, 20, 20}});
    my_menu->addOption({{60, 60, 20, 20}});
    my_menu->addOption({{10, 60, 20, 20}});

    // Start at the first option
    my_menu->jumpTo(0);

    // Create a timer to update the menu
    lv_timer_create(update_timer_cb, 10, nullptr);

    // Simple navigation for demo, you may need to replace with your input device
    lv_obj_t* btn_next = lv_btn_create(scr);
    lv_obj_align(btn_next, LV_ALIGN_BOTTOM_RIGHT, -10, -10);
    lv_obj_t* label_next = lv_label_create(btn_next);
    lv_label_set_text(label_next, "Next");
    lv_obj_add_event_cb(btn_next, [](lv_event_t* e) {
        if (my_menu) my_menu->goNext();
    }, LV_EVENT_CLICKED, nullptr);

    lv_obj_t* btn_prev = lv_btn_create(scr);
    lv_obj_align(btn_prev, LV_ALIGN_BOTTOM_LEFT, 10, -10);
    lv_obj_t* label_prev = lv_label_create(btn_prev);
    lv_label_set_text(label_prev, "Prev");
    lv_obj_add_event_cb(btn_prev, [](lv_event_t* e) {
        if (my_menu) my_menu->goLast();
    }, LV_EVENT_CLICKED, nullptr);
}
