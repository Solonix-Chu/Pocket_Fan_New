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
- [~] Task: Connect to HAL Telemetry
    - [ ] In `homepage::onRunning`, read values from `HAL::get_voltage()`, `HAL::get_current()`, etc.
    - [ ] Update the text of the LVGL labels (e.g., using `lv_label_set_text_fmt`).
    - [ ] Limit updates to every ~200ms to avoid UI stutter.
- [x] Task: Connect to HAL Telemetry 4a5c734
[checkpoint: 55cc048]

## Phase 4: Bug Fixes
- [x] Task: Fix Homepage UI and Navigation Bugs 66e3724
    - [ ] Fix data label visibility (check z-index/parent).
    - [x] Debug button event for Menu transition.

## Phase 5: Polish & Refinement
- [x] Task: Remove Long Press Return Logic 1715005
    - [ ] Remove the `APP_BUTTON_STATE_HOLD` check in `MenuApp`.
- [ ] Task: Implement Quit Option Return
    - [ ] Ensure "Quit" option in Menu triggers return to Homepage (already implemented in `_create_view`, verify logic).
- [ ] Task: Implement Transition Animations
    - [ ] Reference `VAMeter` (Transition3D/Zoom) for app switching.
    - [ ] Implement Zoom In when entering Menu.
    - [ ] Implement Zoom Out when returning to Homepage.

## Phase 6: Final Verification
- [ ] Task: Conductor - User Manual Verification 'Final Verification' (Protocol in workflow.md)
