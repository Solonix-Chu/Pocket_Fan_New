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
    // Simple startup screen
    _screen = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(_screen, lv_color_white(), 0);
    
    lv_obj_t* img = lv_image_create(_screen);
    lv_image_set_src(img, AssetPool::GetImgStartup());
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);

    // Mask for reveal transition
    _mask = lv_obj_create(_screen);
    lv_obj_set_size(_mask, 128, 64);
    lv_obj_clear_flag(_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(_mask, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(_mask, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(_mask, 0, 0);
    lv_obj_set_style_pad_all(_mask, 0, 0);
    lv_obj_set_style_translate_y(_mask, 0, 0);
    
    lv_scr_load(_screen);

    // Prepare transitions
    _backlight.setDurationMs(600);
    _backlight.setEasing(ease::ease_out_back);
    _backlight.jumpTo(0);
    _backlight.moveTo(static_cast<float>(HAL::GetSystemConfig().brightness));
    HAL::SetDisplayBrightness(0);

    _mask_translate.setDurationMs(450);
    _mask_translate.setDelayMs(120);
    _mask_translate.setEasing(ease::ease_out_back);
    _mask_translate.jumpTo(0);
    _mask_translate.moveTo(-80);

    _start_time = HAL::Millis();
    _finish_time = 0;
}

void AppStartupAnim::onRunning()
{
    uint32_t now = HAL::Millis();

    // Animate backlight
    _backlight.updateMs(now);
    float brightness = std::clamp(_backlight.value(), 0.0f, 255.0f);
    HAL::SetDisplayBrightness(static_cast<uint8_t>(brightness));

    // Animate mask reveal
    _mask_translate.updateMs(now);
    if (_mask) {
        lv_obj_set_style_translate_y(_mask, static_cast<lv_coord_t>(_mask_translate.value()), 0);
    }

    if (_backlight.isFinished() && _mask_translate.isFinished()) {
        if (_finish_time == 0) {
            _finish_time = now;
        } else if (now - _finish_time > 300) {
            close();
        }
    }
}

void AppStartupAnim::onClose()
{
    ESP_LOGI(TAG, "onClose");
    if (_screen) {
        lv_obj_del(_screen);
        _screen = nullptr;
    }
    _mask = nullptr;
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
