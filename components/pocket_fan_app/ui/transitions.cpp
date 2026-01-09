#include "transitions.h"
#include <utility>

using namespace smooth_ui_toolkit;

namespace pocket_fan::ui {

namespace {
inline float _ms_to_s(uint32_t ms)
{
    return static_cast<float>(ms) / 1000.0f;
}
} // namespace

TransitionValue::TransitionValue()
    : _ease_fn(ease::ease_out_cubic), _duration_s(0.35f), _delay_s(0.0f)
{
    _apply_options();
}

void TransitionValue::_apply_options()
{
    auto& easing = _anim.easingOptions();
    easing.duration = _duration_s;
    easing.easingFunction = _ease_fn;
    _anim.delay = _delay_s;
}

void TransitionValue::setDurationMs(uint32_t duration_ms)
{
    _duration_s = _ms_to_s(duration_ms);
    _apply_options();
}

void TransitionValue::setDelayMs(uint32_t delay_ms)
{
    _delay_s = _ms_to_s(delay_ms);
    _apply_options();
}

void TransitionValue::setEasing(EaseFn ease_fn)
{
    if (ease_fn) {
        _ease_fn = ease_fn;
        _apply_options();
    }
}

void TransitionValue::setUpdateCallback(std::function<void(TransitionValue&)> cb)
{
    _on_update = std::move(cb);
}

void TransitionValue::setCompleteCallback(std::function<void()> cb)
{
    _anim.onComplete(std::move(cb));
}

void TransitionValue::jumpTo(float value)
{
    _apply_options();
    _anim.teleport(value);
}

void TransitionValue::moveTo(float value)
{
    _apply_options();
    _anim.move(value);
}

void TransitionValue::updateMs(uint32_t now_ms)
{
    _anim.update(_ms_to_s(now_ms));
    if (_on_update) {
        _on_update(*this);
    }
}

float TransitionValue::value()
{
    return _anim.directValue();
}

bool TransitionValue::isFinished()
{
    return _anim.done();
}

Transition2D::Transition2D()
    : _ease_fn(ease::ease_out_cubic), _duration_ms(350), _delay_ms(0)
{
    _apply_options();
}

void Transition2D::_apply_options()
{
    _x.setDurationMs(_duration_ms);
    _y.setDurationMs(_duration_ms);
    _x.setDelayMs(_delay_ms);
    _y.setDelayMs(_delay_ms);
    _x.setEasing(_ease_fn);
    _y.setEasing(_ease_fn);
}

void Transition2D::setDurationMs(uint32_t duration_ms)
{
    _duration_ms = duration_ms;
    _apply_options();
}

void Transition2D::setDelayMs(uint32_t delay_ms)
{
    _delay_ms = delay_ms;
    _apply_options();
}

void Transition2D::setEasing(EaseFn ease_fn)
{
    if (ease_fn) {
        _ease_fn = ease_fn;
        _apply_options();
    }
}

void Transition2D::setUpdateCallback(std::function<void(Transition2D&)> cb)
{
    _on_update = std::move(cb);
}

void Transition2D::jumpTo(float x, float y)
{
    _x.jumpTo(x);
    _y.jumpTo(y);
}

void Transition2D::moveTo(float x, float y)
{
    _x.moveTo(x);
    _y.moveTo(y);
}

void Transition2D::updateMs(uint32_t now_ms)
{
    _x.updateMs(now_ms);
    _y.updateMs(now_ms);
    if (_on_update) {
        _on_update(*this);
    }
}

Vector2 Transition2D::value()
{
    return Vector2(_x.value(), _y.value());
}

bool Transition2D::isFinished()
{
    return _x.isFinished() && _y.isFinished();
}

Transition4D::Transition4D()
    : _ease_fn(ease::ease_out_cubic), _duration_ms(350), _delay_ms(0)
{
    _apply_options();
}

void Transition4D::_apply_options()
{
    _x.setDurationMs(_duration_ms);
    _y.setDurationMs(_duration_ms);
    _w.setDurationMs(_duration_ms);
    _h.setDurationMs(_duration_ms);

    _x.setDelayMs(_delay_ms);
    _y.setDelayMs(_delay_ms);
    _w.setDelayMs(_delay_ms);
    _h.setDelayMs(_delay_ms);

    _x.setEasing(_ease_fn);
    _y.setEasing(_ease_fn);
    _w.setEasing(_ease_fn);
    _h.setEasing(_ease_fn);
}

void Transition4D::setDurationMs(uint32_t duration_ms)
{
    _duration_ms = duration_ms;
    _apply_options();
}

void Transition4D::setDelayMs(uint32_t delay_ms)
{
    _delay_ms = delay_ms;
    _apply_options();
}

void Transition4D::setEasing(EaseFn ease_fn)
{
    if (ease_fn) {
        _ease_fn = ease_fn;
        _apply_options();
    }
}

void Transition4D::setUpdateCallback(std::function<void(Transition4D&)> cb)
{
    _on_update = std::move(cb);
}

void Transition4D::jumpTo(float x, float y, float w, float h)
{
    _x.jumpTo(x);
    _y.jumpTo(y);
    _w.jumpTo(w);
    _h.jumpTo(h);
}

void Transition4D::moveTo(float x, float y, float w, float h)
{
    _x.moveTo(x);
    _y.moveTo(y);
    _w.moveTo(w);
    _h.moveTo(h);
}

void Transition4D::updateMs(uint32_t now_ms)
{
    _x.updateMs(now_ms);
    _y.updateMs(now_ms);
    _w.updateMs(now_ms);
    _h.updateMs(now_ms);
    if (_on_update) {
        _on_update(*this);
    }
}

Vector4 Transition4D::value()
{
    return Vector4(_x.value(), _y.value(), _w.value(), _h.value());
}

bool Transition4D::isFinished()
{
    return _x.isFinished() && _y.isFinished() && _w.isFinished() && _h.isFinished();
}

TransitionColor::TransitionColor()
    : _ease_fn(ease::linear), _duration_ms(300), _delay_ms(0)
{
    _apply_options();
}

void TransitionColor::_apply_options()
{
    const float duration_s = _ms_to_s(_duration_ms);
    const float delay_s = _ms_to_s(_delay_ms);

    _rgb.duration = duration_s;
    _rgb.r_anim.delay = delay_s;
    _rgb.g_anim.delay = delay_s;
    _rgb.b_anim.delay = delay_s;

    _rgb.r_anim.easingOptions().duration = duration_s;
    _rgb.g_anim.easingOptions().duration = duration_s;
    _rgb.b_anim.easingOptions().duration = duration_s;

    _rgb.r_anim.easingOptions().easingFunction = _ease_fn;
    _rgb.g_anim.easingOptions().easingFunction = _ease_fn;
    _rgb.b_anim.easingOptions().easingFunction = _ease_fn;
}

void TransitionColor::setDurationMs(uint32_t duration_ms)
{
    _duration_ms = duration_ms;
    _apply_options();
}

void TransitionColor::setDelayMs(uint32_t delay_ms)
{
    _delay_ms = delay_ms;
    _apply_options();
}

void TransitionColor::setEasing(EaseFn ease_fn)
{
    if (ease_fn) {
        _ease_fn = ease_fn;
        _apply_options();
    }
}

void TransitionColor::setUpdateCallback(std::function<void(TransitionColor&)> cb)
{
    _on_update = std::move(cb);
}

void TransitionColor::jumpTo(uint32_t hex)
{
    _apply_options();
    _rgb.teleport(hex);
}

void TransitionColor::moveTo(uint32_t hex)
{
    _apply_options();
    _rgb.move(hex);
}

void TransitionColor::updateMs(uint32_t now_ms)
{
    const float now_s = _ms_to_s(now_ms);
    _rgb.r_anim.update(now_s);
    _rgb.g_anim.update(now_s);
    _rgb.b_anim.update(now_s);
    _rgb.r = static_cast<uint8_t>(_rgb.r_anim.directValue());
    _rgb.g = static_cast<uint8_t>(_rgb.g_anim.directValue());
    _rgb.b = static_cast<uint8_t>(_rgb.b_anim.directValue());

    if (_on_update) {
        _on_update(*this);
    }
}

color::Rgb_t TransitionColor::value()
{
    return _rgb;
}

uint32_t TransitionColor::valueHex()
{
    return _rgb.toHex();
}

bool TransitionColor::isFinished()
{
    return _rgb.done();
}

} // namespace pocket_fan::ui
