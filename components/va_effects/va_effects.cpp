#include "va_effects.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

using namespace smooth_ui_toolkit;

LGFX_Device* VaEffect::_display = nullptr;
LGFX_Sprite* VaEffect::_canvas = nullptr;
std::function<void()> VaEffect::_feed_watchdog_cb = nullptr;

void VaEffect::Init(LGFX_Device* display, LGFX_Sprite* canvas, std::function<void()> feed_watchdog_cb)
{
    _display = display;
    _canvas = canvas;
    _feed_watchdog_cb = feed_watchdog_cb;
}

void VaEffect::FadeBacklight(float to_brightness, uint32_t duration_ms)
{
    if (!_display) return;

    AnimateValue anim;
    
    // Set start value from current brightness (approximate if not readable)
    // LovyanGFX::getBrightness() might not be available on all devices or return correct value
    // Assuming we can read it or start from a sane default.
    // Ideally the caller should pass start brightness, but preserving API signature:
    anim.start = _display->getBrightness(); 
    anim.end = to_brightness;
    
    // Setup easing
    anim.easingOptions().duration = (float)duration_ms / 1000.0f;
    anim.easingOptions().easingFunction = ease::ease_out_back;
    
    // Start animation
    anim.play();

    while (!anim.done())
    {
        anim.update((float)(esp_timer_get_time() / 1000) / 1000.0f);
        _display->setBrightness((uint8_t)anim.value());
        
        if (_feed_watchdog_cb) _feed_watchdog_cb();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // Ensure final value is set
    _display->setBrightness((uint8_t)to_brightness);
}

void VaEffect::CircularReveal(int start_x, int start_y, int target_x, int target_y, int radius, uint32_t color, uint32_t duration_ms)
{
    if (!_canvas) return;

    AnimateVector2 anim;

    // Setup animation
    anim.teleport(start_x, start_y);
    anim.move(target_x, target_y);
    
    anim.x.easingOptions().duration = (float)duration_ms / 1000.0f;
    anim.x.easingOptions().easingFunction = ease::ease_out_back;
    anim.y.easingOptions().duration = (float)duration_ms / 1000.0f;
    anim.y.easingOptions().easingFunction = ease::ease_out_back;
    
    anim.begin();

    while (!anim.done())
    {
        anim.update((float)(esp_timer_get_time() / 1000) / 1000.0f);
        
        // Clear background? The original code just fills the circle
        // For now, mimic original: just draw expanding circle
        _canvas->fillSmoothRoundRect(
            (int)anim.value().x - radius, 
            (int)anim.value().y - radius, 
            radius * 2, radius * 2, radius, 
            color
        );
        
        _canvas->pushSprite(0, 0);
        if (_feed_watchdog_cb) _feed_watchdog_cb();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // Ensure final position
    _canvas->fillSmoothRoundRect(
        target_x - radius, 
        target_y - radius, 
        radius * 2, radius * 2, radius, 
        color
    );
    _canvas->pushSprite(0, 0);
}

void VaEffect::SlideTransition(Direction direction, int distance, bool is_in, std::function<void(int x, int y)> render_callback, uint32_t duration_ms)
{
    if (!_canvas) return;

    AnimateVector2 anim;
    int start_x = 0, start_y = 0;
    int end_x = 0, end_y = 0;

    switch (direction)
    {
    case Direction::Up:
        start_y = is_in ? distance : 0;
        end_y = is_in ? 0 : -distance;
        break;
    case Direction::Down:
        start_y = is_in ? -distance : 0;
        end_y = is_in ? 0 : distance;
        break;
    case Direction::Left:
        start_x = is_in ? distance : 0;
        end_x = is_in ? 0 : -distance;
        break;
    case Direction::Right:
        start_x = is_in ? -distance : 0;
        end_x = is_in ? 0 : distance;
        break;
    }

    anim.teleport(start_x, start_y);
    anim.move(end_x, end_y);
    
    anim.x.easingOptions().duration = (float)duration_ms / 1000.0f;
    anim.x.easingOptions().easingFunction = ease::ease_out_back;
    anim.y.easingOptions().duration = (float)duration_ms / 1000.0f;
    anim.y.easingOptions().easingFunction = ease::ease_out_back;
    
    anim.begin();

    while (!anim.done())
    {
        anim.update((float)(esp_timer_get_time() / 1000) / 1000.0f);
        
        if (render_callback) render_callback((int)anim.value().x, (int)anim.value().y);
        
        _canvas->pushSprite(0, 0);
        if (_feed_watchdog_cb) _feed_watchdog_cb();
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    // Ensure final position
    if (render_callback) render_callback(end_x, end_y);
    _canvas->pushSprite(0, 0);
}