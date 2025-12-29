# Implementation Plan: Settings Application

## Phase 1: Foundation & Navigation
- [x] Task: Create Settings App Skeleton b1eadce
    - [ ] Create `settings.h/cpp` and `view/settings_view.h/cpp`.
    - [ ] Register `SettingsApp` in `apps.h` and the global `APPS` registry.
- [x] Task: Implement Base List UI e34db43
    - [ ] Create a vertical list using `SmoothSelectorMenu`.
    - [ ] Implement cyclic navigation (wrapping from bottom to top).
    - [ ] Add "Back" option to return to the main Menu app.
- [ ] Task: Conductor - User Manual Verification 'Foundation & Navigation' (Protocol in workflow.md)

## Phase 2: UI Widgets (LVGL V9)
- [ ] Task: Implement Long Text Auto-Scroll
    - [ ] Use `LV_LABEL_LONG_SCROLL_CIRCULAR` for list items.
    - [ ] Configure scroll speed and delay.
- [ ] Task: Implement Theme Toggle & Checkbox
    - [ ] Add square checkbox widget to "Theme" list item.
    - [ ] Implement logic to switch between White and Black themes globally.
- [ ] Task: Implement Brightness Adjustment Popup
    - [ ] Create a centered modal with a progress bar.
    - [ ] Map scroll wheel to 0-100% adjustment.
    - [ ] Apply brightness on OK press and close modal.
- [ ] Task: Conductor - User Manual Verification 'UI Widgets' (Protocol in workflow.md)

## Phase 3: System Logic & Integration
- [ ] Task: Implement Language Switching
    - [ ] Toggle between English and Chinese labels.
    - [ ] Update all active UI components immediately on change.
- [ ] Task: Implement NVS Persistence
    - [ ] Sync changes with `HAL::GetSystemConfig()`.
    - [ ] Call `HAL::SaveSystemConfig()` on every setting change.
- [ ] Task: Integrate into Main Menu
    - [ ] Update `MenuApp` to launch `SettingsApp` when the "Settings" icon is selected.
- [ ] Task: Conductor - User Manual Verification 'System Logic & Integration' (Protocol in workflow.md)
