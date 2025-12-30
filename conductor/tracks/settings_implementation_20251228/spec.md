# Specification: Settings Application Implementation

## Overview
This track involves creating the `settings` application for the Pocket Fan. While the functional logic and UI flow reference the `VAMeter-Firmware` project, the implementation must strictly utilize the **LVGL V9** API and the specific versions of **SmoothUI ToolKit** and **Mooncake** already present in this project's `components/` directory.

## Functional Requirements
- **Core Architecture**:
    - Implement `SettingsApp` (Mooncake App) and `SettingsView` (MVC).
    - Modular sub-views for category management (Display, Language, etc.).
- **User Interface (LVGL V9)**:
    - **Cyclic Scrolling**: A vertical list that wraps around.
    - **Marquee Text**: Use `LV_LABEL_LONG_SCROLL_CIRCULAR` for long menu item names.
    - **Theme Toggle**: Square checkbox widget on the right side of the list item to indicate White/Black theme state.
    - **Brightness Adjustment**: A centered modal popup with a progress indicator. Controlled by the scroll wheel and applied via "OK" button.
    - **Back Action**: Dedicated list item to return to the main Menu app.
- **System Integration**:
    - **Language**: Immediate switch between English and Chinese.
    - **HAL Synchronization**: Direct calls to `HAL::Get()->...` for brightness control and system config updates.
    - **Persistence**: Save changes to NVS using `HAL::SaveSystemConfig()`.

## Technical Constraints
- **Framework**: Use the local `components/mooncake` implementation.
- **UI Library**: Use **LVGL V9**. Ensure all widget calls follow the V9 API (e.g., `lv_image_set_src` instead of `lv_img_set_src`).
- **Animation & Widgets**: Use the local `components/smooth_ui_toolkit`. Adhere to the API established in previous tracks (e.g., `teleport()` for vectors, manual duration setting via `easingOptions()`).

## Acceptance Criteria
- [ ] Settings app integrated into the main Menu and launchable.
- [ ] List scrolling is smooth and cyclic.
- [ ] Long text scrolls automatically within list items.
- [ ] Theme toggles between White/Black and updates UI globally.
- [ ] Brightness popup correctly maps scroll wheel to 0-100% and applies on OK.
- [ ] Language switching applies immediately without restart.
- [ ] All settings persist across device reboots.
