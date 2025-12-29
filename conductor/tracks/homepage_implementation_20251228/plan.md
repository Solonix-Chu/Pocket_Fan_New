# Implementation Plan: Homepage App

## Phase 1: Basic Structure & Navigation
- [x] Task: Create Homepage App Skeleton d3446d8
    - [ ] Create `homepage.h` and `homepage.cpp` in `components/pocket_fan_app/apps/homepage/`.
    - [ ] Implement `onSetup`, `onCreate`, `onDestroy`, etc., inheriting from Mooncake's App interface.
    - [ ] Register the new app in `components/pocket_fan_app/apps/apps.h` and the app loader.
- [~] Task: Implement Background & Page Switching UI
    - [ ] Load background images from assets.
    - [ ] Create an LVGL TileView (`lv_tileview`) to hold the two pages.
    - [ ] Implement the `onRunning` loop to handle wheel input events.
    - [ ] Map wheel scroll to `lv_tileview_set_tile_id` (or similar) with animation.
- [x] Task: Implement Background & Page Switching UI a937e42
- [~] Task: Implement Menu Transition Logic
    - [ ] Detect "OK" button press in `homepage::onRunning`.
    - [ ] Call Mooncake's app manager to switch to the `Menu` app ID.
- [x] Task: Implement Menu Transition Logic e004aa9

## Phase 2: Menu Return Logic (Inactivity & Long Press)
- [x] Task: Implement Menu Inactivity Timer e8fa345
    - [ ] Modify `Menu` app to track the timestamp of the last input event (wheel or button).
    - [ ] In `Menu::onRunning`, check if `millis() - last_input_time > 5000`.
    - [ ] If timed out, switch back to `Homepage` app ID.
- [~] Task: Implement Long Press Return
    - [ ] Modify `Menu` app to detect "Long Press OK" event.
    - [ ] On detection, switch back to `Homepage` app ID.
- [x] Task: Implement Long Press Return 319afec

## Phase 3: Real-Time Data Display
- [x] Task: Layout Data Labels 932f405
    - [ ] Based on `demo/setup_scr_screen.c`, create LVGL labels for V, A, W on the homepage.
    - [ ] Ensure labels are children of the correct tile/page in the TileView.
- [ ] Task: Connect to HAL Telemetry
    - [ ] In `homepage::onRunning`, read values from `HAL::get_voltage()`, `HAL::get_current()`, etc.
    - [ ] Update the text of the LVGL labels (e.g., using `lv_label_set_text_fmt`).
    - [ ] Limit updates to every ~200ms to avoid UI stutter.

## Phase 4: Final Verification
- [ ] Task: Conductor - User Manual Verification 'Final Verification' (Protocol in workflow.md)
