# Specification: Homepage App Implementation

## Overview
This track involves implementing the `homepage` application for the Pocket Fan. The homepage serves as the main dashboard, featuring a dual-page interface with background images, real-time hardware status displays, and navigation logic to the existing `menu` application.

## Functional Requirements
- **Dual-Page Dashboard**:
    - Use images from `components/pocket_fan_app/assets/images/homepage` as full-screen backgrounds.
    - Each image represents a distinct "page" of information.
- **Navigation & Interaction**:
    - **Page Switching**: Use the hall wheel (scroll) to switch between the two pages with a smooth horizontal sliding animation.
    - **Enter Menu**: Press the "OK" button to launch the `menu` application.
    - **Return to Homepage**:
        - In the `menu` app, long-pressing the "OK" button returns to the `homepage`.
        - In the `menu` app, 5 seconds of inactivity (no wheel or button events) triggers an automatic return to the `homepage`.
- **Real-Time Data Display**:
    - Display hardware metrics including Voltage (V), Current (A), Power (W), and Capacity (mAh/Wh).
    - Position metrics according to the layout defined in `demo/setup_scr_screen.c`.
    - Refresh data in real-time (approx. every 100-200ms) using HAL telemetry.

## Technical Details
- **Architecture**: Implement as a Mooncake "App" within the `pocket_fan_app` component.
- **UI Framework**: Use LVGL for the sliding page container (e.g., `lv_tileview` or a custom container with animations) and label updates.
- **Transitions**: Sliding animation for page changes must be fluid.

## Acceptance Criteria
- [ ] Homepage correctly loads both background images.
- [ ] Scrolling the wheel slides between the two homepage pages.
- [ ] Pressing OK enters the Menu app.
- [ ] Long-pressing OK in the Menu app returns to the Homepage.
- [ ] 5 seconds of total inactivity in the Menu app returns to the Homepage.
- [ ] Voltage, Current, and Power data are displayed and updating in real-time on the homepage.
