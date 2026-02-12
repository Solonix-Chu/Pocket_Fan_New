#include "app_startup_anim.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include <esp_log.h>
#include <lvgl.h>
#include <smooth_ui_toolkit.h>
#include <algorithm>

static const char* TAG = "AppStartupAnim";

using namespace smooth_ui_toolkit;

AppStartupAnim::AppStartupAnim()
{
}

AppStartupAnim::~AppStartupAnim()
{
}

bool AppStartupAnim::_guide_active = false;

void AppStartupAnim::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    // Clear display first to avoid showing uninitialized garbage.
    HAL::SetDisplayBrightness(0);
    HAL::SetLedBreath(false);
    HAL::SetLed(0, 0, 0, 0);

    static constexpr uint32_t k_duration_ms = 1200;

    // Simple startup screen (startup photo + progress bar)
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);

    // Restore previous startup image behavior: centered logo on white background.
    lv_obj_t* img = lv_image_create(_screen);
    lv_image_set_src(img, AssetPool::GetImgStartup());
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // Progress bar at bottom for static photo
    _progress = lv_bar_create(_screen);
    lv_bar_set_range(_progress, 0, (int32_t)k_duration_ms);
    lv_bar_set_value(_progress, 0, LV_ANIM_OFF);
    lv_obj_set_size(_progress, 124, 6);
    lv_obj_align(_progress, LV_ALIGN_BOTTOM_MID, 0, -2);
    lv_obj_set_style_radius(_progress, 2, 0);
    lv_obj_set_style_border_width(_progress, 1, 0);
    lv_obj_set_style_border_color(_progress, lv_color_black(), 0);
    lv_obj_set_style_bg_color(_progress, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_progress, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(_progress, lv_color_black(), LV_PART_INDICATOR);
    
    lv_scr_load(_screen);

    _start_time = HAL::Millis();
}

void AppStartupAnim::onRunning()
{
    uint32_t now = HAL::Millis();
    static constexpr uint32_t k_duration_ms = 1200;

    // Turn on backlight after first tick to avoid any visible garbage before the first flush.
    if (now - _start_time >= 20) {
        HAL::SetDisplayBrightness(static_cast<uint8_t>(HAL::GetSystemConfig().brightness));
    }

    // Progress -> homepage
    uint32_t elapsed = now - _start_time;
    if (_progress) {
        if (elapsed > k_duration_ms) elapsed = k_duration_ms;
        lv_bar_set_value(_progress, (int32_t)elapsed, LV_ANIM_OFF);
    }

    // Trackball LED: white slow fade-in synced with startup duration.
    uint32_t fade_elapsed = (now - _start_time);
    if (fade_elapsed > k_duration_ms) fade_elapsed = k_duration_ms;
    uint8_t w = (uint8_t)((fade_elapsed * 255U) / k_duration_ms);
    HAL::SetLed(0, 0, 0, w);

    if (elapsed >= k_duration_ms) {
        close();
    }
}

void AppStartupAnim::onClose()
{
    ESP_LOGI(TAG, "onClose");
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
        _progress = nullptr;
    }
}

void AppStartupAnim::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
}

void AppStartupAnim::PopUpGuideMap(bool force)
{
    if (!force) return;
    if (_guide_active) {
        ESP_LOGW(TAG, "Guide already active, skip");
        return;
    }
    _guide_active = true;

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
    const uint32_t wait_start = HAL::Millis();
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
        // Auto close after 2.5s to avoid blocking / WDT
        if (HAL::Millis() - wait_start > 2500) {
            ESP_LOGW(TAG, "Guide auto-dismiss due to timeout");
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
    _guide_active = false;
}
