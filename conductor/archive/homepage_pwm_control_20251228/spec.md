# Specification: Homepage PWM Control

## Overview
This track adds real-time PWM (fan speed) control directly to the first page of the Homepage dashboard. Users will be able to adjust the fan power from 0% to 100% using the vertical scroll wheel (Up/Down) without leaving the main screen.

## Functional Requirements
- **PWM Display**: 
    - Add a text label on the right side of Homepage 1 showing the current PWM duty cycle (e.g., "50%").
    - Use the established black text on white background theme.
- **Adaptive Adjustment**:
    - Use the `BTN_UP` and `BTN_DOWN` events (scroll wheel) to increase or decrease the PWM value.
    - **Dynamic Step Size:** The rate of change correlates with scrolling speed. Faster scrolling results in larger step increments, while slow scrolling allows for precise 1% adjustments.
    - Range: 0% to 100%.
- **Hardware Integration**:
    - Call `HAL::SetFanSpeed(float)` to apply the new duty cycle in real-time.
    - (Optional) Persist the last speed setting if supported by the system configuration.

## Technical Details
- **Architecture**:
    - Update `HomepageView` to include the PWM label and an `updatePwm(int)` method.
    - Update `HomepageApp::onRunning` to detect `BTN_UP`/`BTN_DOWN`.
    - Implement a speed detection algorithm (e.g., measuring time delta between input events) to calculate the dynamic step size.
- **UI Framework**: Use an LVGL label parented to the first tile of the dashboard.

## Acceptance Criteria
- [ ] A PWM percentage label is visible on the right side of Homepage Page 1.
- [ ] Scrolling UP/DOWN adjusts the fan speed.
- [ ] Slow scrolling changes the value by 1%.
- [ ] Fast scrolling changes the value by larger increments (e.g., 5% or 10%).
- [ ] The value is capped at 0 and 100.
- [ ] The fan hardware responds immediately to the UI changes.
