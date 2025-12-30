#include "app_startup_anim.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include <esp_log.h>
#include <lvgl.h>
#include <smooth_ui_toolkit.h>

static const char* TAG = "AppStartupAnim";

using namespace smooth_ui_toolkit;

AppStartupAnim::AppStartupAnim()
{
}

AppStartupAnim::~AppStartupAnim()
{
}

void AppStartupAnim::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    // Simple startup screen
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    
    lv_obj_t* img = lv_image_create(_screen);
    lv_image_set_src(img, AssetPool::GetImgStartup());
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
    
    lv_scr_load(_screen);
    _start_time = HAL::Millis();
}

void AppStartupAnim::onRunning()
{
    if (HAL::Millis() - _start_time > 1500) {
        close();
    }
}

void AppStartupAnim::onClose()
{
    ESP_LOGI(TAG, "onClose");
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
}

void AppStartupAnim::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
}

void AppStartupAnim::PopUpGuideMap(bool force)
{
    if (!force) return;

    ESP_LOGI(TAG, "PopUpGuideMap");

    AnimateVector2 transition;
    transition.x.easingOptions().easingFunction = ease::ease_out_back;
    transition.y.easingOptions().easingFunction = ease::ease_out_back;
    
    transition.x.easingOptions().duration = 0.6f;
    transition.y.easingOptions().duration = 0.6f;
    
    transition.teleport(0, 64); // Start below screen
    transition.move(0, 0);

    // Create a temporary screen overlay or just draw on top of current?
    // The reference pushes image to HAL Canvas. We are using LVGL.
    // We should create an LVGL object (image or container) on the current active screen (top layer).

    lv_obj_t* layer_top = lv_layer_top();
    lv_obj_t* popup_img = lv_image_create(layer_top);
    
    // Use a placeholder image if guide map is not available
    // Using HomePage1 as placeholder
    lv_image_set_src(popup_img, AssetPool::GetImgHomePage1());
    // lv_obj_align(popup_img, LV_ALIGN_CENTER, 0, 0); // Remove align to avoid conflict with set_pos
    
    // We will animate its Y position using loop
    
    // Sync time before starting animation
    transition.update(HAL::Millis() / 1000.0f);
    
    transition.teleport(0, 64); // Start below screen
    transition.move(0, 0);

    uint32_t start_time = HAL::Millis();
    (void)start_time; // Suppress unused warning

    while (!transition.done())
    {
        transition.update(HAL::Millis() / 1000.0f);
        lv_obj_set_pos(popup_img, (int32_t)transition.value().x, (int32_t)transition.value().y);
        
        lv_timer_handler(); // Process LVGL
        HAL::AllButtonRefresh();
        HAL::FeedTheDog();
        HAL::Delay(5);
    }

    // Wait for input to dismiss
    while (1)
    {
        HAL::Delay(50);
        lv_timer_handler();
        HAL::AllButtonRefresh();
        HAL::FeedTheDog();

        if (HAL::GetButton(BUTTON::BTN_MID) == APP_BUTTON_STATE_CLICKED || 
            HAL::GetButton(BUTTON::BTN_LEFT) == APP_BUTTON_STATE_CLICKED) {
            // Dismiss anim
            break;
        }
    }
    
    // Dismiss animation
    // Sync time again before new move
    transition.update(HAL::Millis() / 1000.0f);
    transition.move(0, 64);
    
    while (!transition.done())
    {
        transition.update(HAL::Millis() / 1000.0f);
        lv_obj_set_pos(popup_img, (int32_t)transition.value().x, (int32_t)transition.value().y);
        
        lv_timer_handler();
        HAL::AllButtonRefresh();
        HAL::FeedTheDog();
        HAL::Delay(5);
    }

    lv_obj_del(popup_img);
}
