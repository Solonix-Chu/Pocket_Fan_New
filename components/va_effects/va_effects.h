#pragma once

#include <smooth_ui_toolkit.h>
#include <LovyanGFX.hpp>
#include <vector>
#include <functional>

/**
 * @brief VAMeter Animation Kit
 * 
 * A utility factory/helper class to provide reusable animation effects
 * ported from the VAMeter firmware.
 */
class VaEffect
{
public:
    // Prevent instantiation
    VaEffect() = delete;

    /**
     * @brief Initialize the animation kit with hardware dependencies
     * 
     * @param display Pointer to the LovyanGFX display device
     * @param canvas Pointer to the drawing canvas (Sprite)
     * @param feed_watchdog_cb Optional callback to feed watchdog during blocking animations
     */
    static void Init(LGFX_Device* display, LGFX_Sprite* canvas, std::function<void()> feed_watchdog_cb = nullptr);

    /**
     * @brief Smoothly change backlight brightness
     * 
     * @param to_brightness Target brightness (0-255)
     * @param duration_ms Duration in milliseconds
     */
    static void FadeBacklight(float to_brightness, uint32_t duration_ms);

    /**
     * @brief Circular mask expansion reveal
     * 
     * @param start_x Start X position
     * @param start_y Start Y position
     * @param target_x Target X position (usually screen center)
     * @param target_y Target Y position (usually screen center)
     * @param radius Circle radius
     * @param color Fill color
     * @param duration_ms Duration in milliseconds
     */
    static void CircularReveal(int start_x, int start_y, int target_x, int target_y, int radius, uint32_t color, uint32_t duration_ms);

    enum class Direction {
        Up, Down, Left, Right
    };

    /**
     * @brief Slide an image/callback into or out of view
     * 
     * @param direction Slide direction
     * @param distance Slide distance
     * @param is_in True for SlideIn, false for SlideOut
     * @param render_callback Callback to render the content at (x, y)
     * @param duration_ms Duration in milliseconds
     */
    static void SlideTransition(Direction direction, int distance, bool is_in, std::function<void(int x, int y)> render_callback, uint32_t duration_ms);

private:
    static LGFX_Device* _display;
    static LGFX_Sprite* _canvas;
    static std::function<void()> _feed_watchdog_cb;
};