#pragma once
#include <functional>
#include <cstdint>
#include "smooth_ui_toolkit.h"

namespace pocket_fan::ui {

using EaseFn = float (*)(float);

class TransitionValue {
public:
    TransitionValue();

    void setDurationMs(uint32_t duration_ms);
    void setDelayMs(uint32_t delay_ms);
    void setEasing(EaseFn ease_fn);
    void setUpdateCallback(std::function<void(TransitionValue&)> cb);
    void setCompleteCallback(std::function<void()> cb);

    void jumpTo(float value);
    void moveTo(float value);
    void updateMs(uint32_t now_ms);

    float value();
    bool isFinished();

private:
    void _apply_options();

    smooth_ui_toolkit::AnimateValue _anim;
    EaseFn _ease_fn;
    float _duration_s;
    float _delay_s;
    std::function<void(TransitionValue&)> _on_update;
};

class Transition2D {
public:
    Transition2D();

    void setDurationMs(uint32_t duration_ms);
    void setDelayMs(uint32_t delay_ms);
    void setEasing(EaseFn ease_fn);
    void setUpdateCallback(std::function<void(Transition2D&)> cb);

    void jumpTo(float x, float y);
    void moveTo(float x, float y);
    void updateMs(uint32_t now_ms);

    smooth_ui_toolkit::Vector2 value();
    bool isFinished();

private:
    void _apply_options();

    TransitionValue _x;
    TransitionValue _y;
    EaseFn _ease_fn;
    uint32_t _duration_ms;
    uint32_t _delay_ms;
    std::function<void(Transition2D&)> _on_update;
};

class Transition4D {
public:
    Transition4D();

    void setDurationMs(uint32_t duration_ms);
    void setDelayMs(uint32_t delay_ms);
    void setEasing(EaseFn ease_fn);
    void setUpdateCallback(std::function<void(Transition4D&)> cb);

    void jumpTo(float x, float y, float w, float h);
    void moveTo(float x, float y, float w, float h);
    void updateMs(uint32_t now_ms);

    smooth_ui_toolkit::Vector4 value();
    bool isFinished();

private:
    void _apply_options();

    TransitionValue _x;
    TransitionValue _y;
    TransitionValue _w;
    TransitionValue _h;
    EaseFn _ease_fn;
    uint32_t _duration_ms;
    uint32_t _delay_ms;
    std::function<void(Transition4D&)> _on_update;
};

class TransitionColor {
public:
    TransitionColor();

    void setDurationMs(uint32_t duration_ms);
    void setDelayMs(uint32_t delay_ms);
    void setEasing(EaseFn ease_fn);
    void setUpdateCallback(std::function<void(TransitionColor&)> cb);

    void jumpTo(uint32_t hex);
    void moveTo(uint32_t hex);
    void updateMs(uint32_t now_ms);

    smooth_ui_toolkit::color::Rgb_t value();
    uint32_t valueHex();
    bool isFinished();

private:
    void _apply_options();

    smooth_ui_toolkit::color::AnimateRgb_t _rgb;
    EaseFn _ease_fn;
    uint32_t _duration_ms;
    uint32_t _delay_ms;
    std::function<void(TransitionColor&)> _on_update;
};

} // namespace pocket_fan::ui
