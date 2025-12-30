# Implementation Plan: Homepage PWM Control

## Phase 1: UI Implementation
- [x] Task: Add PWM Label to HomepageView 106768b
    - [ ] Add `lv_obj_t* _label_pwm` to `HomepageView`.
    - [ ] Initialize the label on Page 1 (right side, black text, white bg).
    - [ ] Implement `HomepageView::updatePwm(int dutyCycle)` to update the label text.

## Phase 2: Logic & Control
- [x] Task: Implement PWM Control Logic in HomepageApp dc06c3c
    - [ ] Add state variables: `_fan_speed` (int, 0-100), `_last_scroll_time` (uint32_t).
    - [ ] In `onRunning`, detect `BTN_UP` and `BTN_DOWN`.
    - [ ] Implement adaptive step calculation based on `HAL::Millis() - _last_scroll_time`.
    - [ ] Update `_fan_speed` and call `HAL::SetFanSpeed` and `_view->updatePwm`.
    - [ ] Ensure `HAL::SetFanState(true)` is called if speed > 0.

## Phase 3: Verification
- [x] Task: Conductor - User Manual Verification 'Final Verification' (Protocol in workflow.md) f878930
[checkpoint: f878930]
