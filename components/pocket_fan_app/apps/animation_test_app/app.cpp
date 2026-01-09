#include "app.h"
#include "../../hal/hal.h"
#include "../../ui/transitions.h"
#include "../../assets/assets.h"
#include <va_effects.h>
#include <LovyanGFX.hpp>
#include <app_button.h>
#include <vector>

static lv_group_t* _group = nullptr;
namespace {
enum class DemoMode {
    None,
    StaggerList,
    PanelSlide,
    MaskReveal,
};

struct DemoContext {
    DemoMode mode = DemoMode::None;
    lv_obj_t* overlay = nullptr;
    std::vector<lv_obj_t*> items;
    std::vector<pocket_fan::ui::Transition2D> item_transitions;
    std::vector<int> item_base_y;
    pocket_fan::ui::TransitionValue panel_transition;
    pocket_fan::ui::TransitionValue mask_transition;
    uint32_t start_ms = 0;
    bool exit_triggered = false;
};

DemoContext g_demo;

void clear_demo()
{
    if (g_demo.overlay) {
        lv_obj_del(g_demo.overlay);
    }
    g_demo.overlay = nullptr;
    g_demo.items.clear();
    g_demo.item_transitions.clear();
    g_demo.item_base_y.clear();
    g_demo.exit_triggered = false;
    g_demo.mode = DemoMode::None;
}

void start_stagger_list(lv_obj_t* parent)
{
    clear_demo();
    g_demo.mode = DemoMode::StaggerList;
    g_demo.start_ms = HAL::Millis();

    g_demo.overlay = lv_obj_create(parent);
    lv_obj_set_size(g_demo.overlay, 128, 64);
    lv_obj_set_style_bg_color(g_demo.overlay, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(g_demo.overlay, LV_OPA_20, 0); // faint white overlay on black
    lv_obj_set_style_border_width(g_demo.overlay, 0, 0);
    lv_obj_clear_flag(g_demo.overlay, LV_OBJ_FLAG_SCROLLABLE);

    const char* labels[] = {"Voltage", "Current", "Power", "Temp"};
    const int label_count = sizeof(labels) / sizeof(labels[0]);
    g_demo.items.reserve(label_count);
    g_demo.item_transitions.resize(label_count);
    g_demo.item_base_y.resize(label_count);

    for (int i = 0; i < label_count; ++i) {
        int y = 6 + i * 14;
        g_demo.item_base_y[i] = y;

        lv_obj_t* lbl = lv_label_create(g_demo.overlay);
        lv_label_set_text(lbl, labels[i]);
        lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
        lv_obj_set_style_text_font(lbl, AssetPool::GetGullyBold12(), 0);
        lv_obj_set_pos(lbl, 4, y);
        lv_obj_set_style_translate_x(lbl, -140, 0);
        g_demo.items.push_back(lbl);

        auto& t = g_demo.item_transitions[i];
        t.setDurationMs(420);
        t.setDelayMs(60 * i);
        t.setEasing(smooth_ui_toolkit::ease::ease_out_back);
        t.jumpTo(-140, y);
        t.moveTo(0, y);
    }
}

void start_panel_slide(lv_obj_t* parent)
{
    clear_demo();
    g_demo.mode = DemoMode::PanelSlide;
    g_demo.start_ms = HAL::Millis();

    g_demo.overlay = lv_obj_create(parent);
    lv_obj_set_size(g_demo.overlay, 128, 64);
    lv_obj_set_style_bg_opa(g_demo.overlay, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(g_demo.overlay, 0, 0);
    lv_obj_clear_flag(g_demo.overlay, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* panel = lv_obj_create(g_demo.overlay);
    lv_obj_set_size(panel, 128, 30);
    lv_obj_set_style_bg_color(panel, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_set_style_radius(panel, 6, 0);
    lv_obj_set_style_translate_y(panel, 80, 0);
    lv_obj_align(panel, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t* lbl = lv_label_create(panel);
    lv_label_set_text(lbl, "Panel Slide");
    lv_obj_set_style_text_color(lbl, lv_color_black(), 0);
    lv_obj_center(lbl);

    g_demo.panel_transition.setDurationMs(420);
    g_demo.panel_transition.setEasing(smooth_ui_toolkit::ease::ease_out_back);
    g_demo.panel_transition.jumpTo(80);
    g_demo.panel_transition.moveTo(0);
    g_demo.items.push_back(panel);
}

void start_mask_reveal(lv_obj_t* parent)
{
    clear_demo();
    g_demo.mode = DemoMode::MaskReveal;
    g_demo.start_ms = HAL::Millis();

    g_demo.overlay = lv_obj_create(parent);
    lv_obj_set_size(g_demo.overlay, 128, 64);
    lv_obj_set_style_bg_color(g_demo.overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(g_demo.overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(g_demo.overlay, 0, 0);
    lv_obj_clear_flag(g_demo.overlay, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_translate_y(g_demo.overlay, 64, 0);

    g_demo.mask_transition.setDurationMs(450);
    g_demo.mask_transition.setEasing(smooth_ui_toolkit::ease::ease_out_back);
    g_demo.mask_transition.jumpTo(64);
    g_demo.mask_transition.moveTo(0);
}

void update_demo(uint32_t now_ms)
{
    switch (g_demo.mode) {
    case DemoMode::None:
        return;
    case DemoMode::StaggerList: {
        bool all_done = true;
        for (size_t i = 0; i < g_demo.item_transitions.size(); ++i) {
            g_demo.item_transitions[i].updateMs(now_ms);
            if (i < g_demo.items.size()) {
                lv_obj_set_style_translate_x(
                    g_demo.items[i],
                    (lv_coord_t)g_demo.item_transitions[i].value().x,
                    0);
            }
            all_done = all_done && g_demo.item_transitions[i].isFinished();
        }

        if (!g_demo.exit_triggered && all_done && now_ms - g_demo.start_ms > 1300) {
            g_demo.exit_triggered = true;
            for (size_t i = 0; i < g_demo.item_transitions.size(); ++i) {
                g_demo.item_transitions[i].setDurationMs(260);
                g_demo.item_transitions[i].setDelayMs(30 * i);
                g_demo.item_transitions[i].setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
                g_demo.item_transitions[i].moveTo(-140, g_demo.item_base_y[i]);
            }
        }

        if (g_demo.exit_triggered) {
            bool finished = true;
            for (auto& t : g_demo.item_transitions) {
                t.updateMs(now_ms);
                finished = finished && t.isFinished();
            }
            if (finished) {
                clear_demo();
            }
        }
        break;
    }
    case DemoMode::PanelSlide: {
        if (!g_demo.items.empty()) {
            auto* panel = g_demo.items.front();
            g_demo.panel_transition.updateMs(now_ms);
            lv_obj_set_style_translate_y(panel, (lv_coord_t)g_demo.panel_transition.value(), 0);
        }

        if (!g_demo.exit_triggered && g_demo.panel_transition.isFinished() && now_ms - g_demo.start_ms > 1200) {
            g_demo.exit_triggered = true;
            g_demo.panel_transition.setDurationMs(260);
            g_demo.panel_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
            g_demo.panel_transition.moveTo(80);
        } else if (g_demo.exit_triggered && g_demo.panel_transition.isFinished()) {
            clear_demo();
        }
        break;
    }
    case DemoMode::MaskReveal: {
        g_demo.mask_transition.updateMs(now_ms);
        if (g_demo.overlay) {
            lv_obj_set_style_translate_y(g_demo.overlay, (lv_coord_t)g_demo.mask_transition.value(), 0);
        }

        if (!g_demo.exit_triggered && g_demo.mask_transition.isFinished() && now_ms - g_demo.start_ms > 700) {
            g_demo.exit_triggered = true;
            g_demo.mask_transition.setDurationMs(260);
            g_demo.mask_transition.setEasing(smooth_ui_toolkit::ease::ease_out_cubic);
            g_demo.mask_transition.moveTo(-64);
        } else if (g_demo.exit_triggered && g_demo.mask_transition.isFinished()) {
            clear_demo();
        }
        break;
    }
    }
}
} // namespace

AnimationTestApp::AnimationTestApp()
{
}

static void btn_event_cb(lv_event_t * e)
{
    auto* self = static_cast<AnimationTestApp*>(lv_event_get_user_data(e));
    lv_obj_t * btn = (lv_obj_t *)lv_event_get_target(e);
    const char * label = lv_label_get_text(lv_obj_get_child(btn, 0));

    if (strcmp(label, "Fade Out") == 0) {
        VaEffect::FadeBacklight(20, 1000);
    } else if (strcmp(label, "Fade In") == 0) {
        VaEffect::FadeBacklight(200, 1000);
    } else if (strcmp(label, "Circle Reveal") == 0) {
        // Solid white reveal for monochrome panel
        VaEffect::CircularReveal(0, 0, 64, 32, 100, 0xFFFF, 1000);
    } else if (strcmp(label, "Slide In") == 0) {
        // Monochrome slide-in: white panel on black background
        VaEffect::SlideTransition(VaEffect::Direction::Up, 64, true, [](int x, int y) {
            HAL::GetCanvas()->fillRect(x, y, 128, 64, 0xFFFF);
            HAL::GetCanvas()->setTextColor(0x0000);
            HAL::GetCanvas()->drawString("Sliding In...", x + 14, y + 25);
        }, 1000);
    } else if (strcmp(label, "Stagger List") == 0) {
        if (self) self->startStaggerListDemo();
    } else if (strcmp(label, "Panel Slide") == 0) {
        if (self) self->startPanelSlideDemo();
    } else if (strcmp(label, "Mask Reveal") == 0) {
        if (self) self->startMaskRevealDemo();
    }
}

void AnimationTestApp::onOpen()
{
    // Initialize VaEffect with HAL
    VaEffect::Init(HAL::GetDisplay(), (LGFX_Sprite*)HAL::GetCanvas(), [](){ HAL::FeedTheDog(); });

    // Initialize screen
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_black(), 0);
    lv_scr_load(_screen);

    // Create group for navigation
    _group = lv_group_create();
    lv_group_set_default(_group);

    // List for gallery
    lv_obj_t * list = lv_list_create(_screen);
    lv_obj_set_size(list, 128, 64);
    lv_obj_center(list);
    lv_obj_set_style_bg_color(list, lv_color_black(), 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    const char * btn_labels[] = {"Fade Out", "Fade In", "Circle Reveal", "Slide In", "Stagger List", "Panel Slide", "Mask Reveal"};
    for (const char * lbl : btn_labels) {
        lv_obj_t * btn = lv_list_add_btn(list, NULL, lbl);
        lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, this);
        lv_group_add_obj(_group, btn);
        
        // Style for focused state
        lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), LV_STATE_FOCUSED);
        lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, LV_STATE_FOCUSED);
        lv_obj_set_style_text_color(btn, lv_color_white(), LV_STATE_FOCUSED);
    }
}

void AnimationTestApp::onRunning()
{
    // Key navigation using global button pointers (mimicking MenuView)
    if (BtnDown && BtnDown->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        lv_group_focus_next(_group);
    }
    else if (BtnUp && BtnUp->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        lv_group_focus_prev(_group);
    }
    else if (BtnOk && BtnOk->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        lv_obj_t* focused = lv_group_get_focused(_group);
        if (focused) {
            lv_obj_send_event(focused, LV_EVENT_CLICKED, NULL);
        }
    }

    // Check for back button (hardware long press)
    if (BtnOk && BtnOk->currentState == APP_BUTTON_STATE_HOLD) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        close();
    }

    update_demo(HAL::Millis());
}

void AnimationTestApp::onClose()
{
    clear_demo();

    if (_group) {
        lv_group_del(_group);
        _group = nullptr;
    }
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void AnimationTestApp::onDestroy()
{
    // Cleanup
}

// Static helpers exposed for button callbacks
void AnimationTestApp::startStaggerListDemo()
{
    if (_screen) {
        start_stagger_list(_screen);
    }
}

void AnimationTestApp::startPanelSlideDemo()
{
    if (_screen) {
        start_panel_slide(_screen);
    }
}

void AnimationTestApp::startMaskRevealDemo()
{
    if (_screen) {
        start_mask_reveal(_screen);
    }
}
