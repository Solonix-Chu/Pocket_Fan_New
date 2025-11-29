#include "launcher.h"
#include "app_button.h"
#include "../../asset/assets.h"

void LauncherApp::onLauncherCreate()
{
    printf("Launcher Create\n");
}

void LauncherApp::_create_ui()
{
    if (_screen) return;
    _screen = lv_obj_create(NULL); // Create a screen
    lv_obj_set_style_bg_color(_screen, lv_color_white(), LV_PART_MAIN);
    // lv_obj_set_style_bg_opa(_screen, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(_screen, lv_color_black(), 0);
    lv_obj_clear_flag(_screen, LV_OBJ_FLAG_SCROLLABLE);
    
    // Menu Container to hold items and move with camera
    lv_obj_t* menu_cont = lv_obj_create(_screen);
    lv_obj_set_size(menu_cont, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(menu_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(menu_cont, 0, 0);
    lv_obj_clear_flag(menu_cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // Selector (inside menu container)
    _selector_obj = lv_obj_create(menu_cont);
    lv_obj_set_style_bg_color(_selector_obj, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(_selector_obj, LV_OPA_TRANSP, 0); // Semi-transparent
    lv_obj_set_style_radius(_selector_obj, 5, 0);
    lv_obj_set_style_border_width(_selector_obj, 1, 0);
    lv_obj_set_style_border_color(_selector_obj, lv_color_black(), 0);
    lv_obj_clear_flag(_selector_obj, LV_OBJ_FLAG_SCROLLABLE);

    // Get apps
    auto app_list = getAppProps();
    _app_ids.clear();
    _option_names.clear();
    _data.option_list.clear(); // Reset SmoothMenu options
    _option_labels.clear();

    // Icon resources
    // const lv_image_dsc_t* icons[] = {
    //     &_1_RGB565A8_34x25, &_2_RGB565A8_34x26, &_3_RGB565A8_34x26,
    //     &_4_RGB565A8_34x26, &_5_RGB565A8_34x26, &_6_RGB565A8_34x26
    // };
    // int icon_count = 6;

    // Horizontal Layout
    // Screen 128x64. 
    // We want 3 items visible. Center item at screen center (64).
    // Item pitch approx 42px.
    // Let's start x so that index 0 is centered when initialized? 
    // SmoothSelectorMenu usually puts the selector at the keyframe. 
    // We can just lay them out linearly: 0, 45, 90, 135...
    // The camera will handle the view.
    int x = 10; 
    int y = 12; // Centered vertically roughly (64 - 40)/2
    int i = 0;

    for (const auto& app : app_list) {
        _app_ids.push_back(app.appID);
        _option_names.push_back(app.info.name);
        
        // Add option to menu
        // Keyframe defines the target selector position/shape for this item
        Option_t option;
        option.keyframe = {(float)x, (float)y, 44, 40}; // Width 44 (icon 34 + padding), Height 40
        addOption(option);
        
        // Create UI Element (Label instead of Image for Monochrome)
        lv_obj_t* container = lv_obj_create(menu_cont);
        lv_obj_set_size(container, 34, 26); // Match icon size
        lv_obj_set_style_bg_color(container, lv_color_black(), 0);
        lv_obj_set_style_border_width(container, 1, 0);
        lv_obj_set_style_border_color(container, lv_color_white(), 0);
        lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);
        
        // Add Text/Symbol inside
        lv_obj_t* label = lv_label_create(container);
        lv_label_set_text(label, app.info.name.c_str()); 
        // Or use symbols if you want: lv_label_set_text(label, LV_SYMBOL_SETTINGS);
        lv_obj_center(label);

        // Align container to center of the keyframe slot
        lv_obj_align(container, LV_ALIGN_TOP_LEFT, x + 5, y + 7);
        _option_labels.push_back(container);
        
        // Optional: Add text label below?
        // For now, just icons as per "reference the interface design" (which had no text)
        
        x += 45; // Pitch
        i++;
    }
    
    // Setup SmoothMenu
    // Camera size matches screen size.
    setConfig().cameraSize = {128, 64};
    setConfig().renderInterval = 10;
    setConfig().readInputInterval = 20;
    setConfig().moveInLoop = true;
    
    if (!_data.option_list.empty()) {
        jumpTo(0);
    }
}

void LauncherApp::onLauncherOpen()
{
    printf("Launcher Open\n");
    _create_ui();
    lv_scr_load(_screen);
}

void LauncherApp::onLauncherRunning()
{
    update(); // Calls onReadInput, onRender
}

void LauncherApp::onLauncherClose()
{
    printf("Launcher Close");
    // We might want to keep the UI object but hide it, or destroy it to save RAM.
    // Mooncake apps are destroyed when uninstalled, but kept when closed/sleeping?
    // Standard Mooncake: sleeping apps are just not running.
    // LVGL: if we load another screen, this one is just in BG.
    // But to save RAM, let's destroy.
    _destroy_ui();
}

void LauncherApp::onLauncherSleeping()
{
    // Nothing
}

void LauncherApp::onLauncherDestroy()
{
    _destroy_ui();
}

void LauncherApp::_destroy_ui()
{
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        _selector_obj = nullptr;
        _option_labels.clear();
    }
}

void LauncherApp::onReadInput()
{
    // Check AppButton
    if (BtnDown->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
        goNext();
    } else if (BtnUp->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
        goLast();
    } else if (BtnOk->currentState == APP_BUTTON_STATE_CLICKED) {
        BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
        // Squeeze animation
        auto current = getSelectorCurrentFrame();
        press({current.x + 5, current.y + 5, current.width - 10, current.height - 10});
    } else if (BtnOk->currentState == APP_BUTTON_STATE_NOCHANGE && isPressing()) {
        release();
    }
}

void LauncherApp::onRender()
{
    if (!_screen) return;
    
    // Update Selector
    auto selector_frame = getSelectorCurrentFrame();
    lv_obj_set_pos(_selector_obj, selector_frame.x, selector_frame.y);
    lv_obj_set_size(_selector_obj, selector_frame.width, selector_frame.height);
    
    // Update Camera (Move container)
    // Goal: Keep the selector centered on the screen (128x64)
    auto camera = getCameraOffset();
    
    // Screen Center is 64, 32.
    // We want the center of the selector to be at the center of the screen.
    // Selector Center X = camera.x + selector_frame.width / 2
    // Selector Center Y = camera.y + selector_frame.height / 2
    // Menu Pos X = Screen Center X - Selector Center X
    
    // Note: SmoothSelectorMenu camera usually tracks the top-left of the selector (or whatever keyframe x,y is).
    // Let's assume camera.x/y is the current top-left of the selector.
    
    lv_obj_t* menu_cont = lv_obj_get_parent(_selector_obj);
    lv_obj_set_pos(menu_cont, -camera.x, -camera.y);
}

void LauncherApp::onClick()
{
    int selected = getSelectedOptionIndex();
    if (selected >= 0 && selected < _app_ids.size()) {
        openApp(_app_ids[selected]);
    }
}
