# Specification: VAMeter Animation Port & Animation Kit Component

## 1. Overview
This track involves analyzing the animation logic from the VAMeter firmware and porting it to the current project as a standalone component. Due to version differences in `smooth_ui_toolkit`, the animations will be encapsulated into a new "Animation Kit" utility component to provide reusable APIs for page transitions, menu effects, and data visualization animations. A comprehensive test application will be created to verify the implementation.

## 2. Functional Requirements

### 2.1 Animation Kit Component (`va_effects`)
- **API Design**: Implement a utility-based API (Factory/Helper pattern) that can be applied to any `smooth_ui_toolkit` widget or page.
- **Ported Animations**:
    - **Page Transitions**: Circular mask expansion (mask reveal), vertical/horizontal slide-in, and backlight brightness fading.
    - **Menu Effects**: Staggered cascading entrance for list items, panel "bounce" entrance, and selector "squeeze" feedback.
    - **Data Visuals**: Smooth chart axis zooming/panning and staggered value entrance (sliding from off-screen).
    - **Widgets**: Rotating spinners and expanding notification bubbles.

### 2.2 Comprehensive Animation Test App
- **Gallery Mode**: A selection menu to view each animation type in isolation.
- **Interactive Playground**: Real-time adjustment of animation parameters (duration, easing paths).
- **Stress Test**: Simultaneous execution of multiple animations to monitor performance.
- **Mock Apps**: Lightweight reconstructions of VAMeter-style Launcher and Waveform views to demonstrate the API in context.

## 3. Non-Functional Requirements
- **Performance**: Animations must maintain a stable frame rate on the target hardware.
- **Modularity**: The new component should depend on `smooth_ui_toolkit` but remain independent of specific application logic.
- **Style**: Mimic the visual "smoothness" and easing (mostly `easeOutBack`) of the original VAMeter firmware.

## 4. Acceptance Criteria
- A new component (e.g., `components/va_effects`) is created and builds successfully.
- All identified animation types from VAMeter are accessible via the new API.
- The Animation Test App is fully functional and covers all test modes (Gallery, Playground, Stress, Mocks).
- Easing paths match the original VAMeter behavior (verified via visual inspection).

## 5. Out of Scope
- Porting non-visual logic (e.g., actual power monitoring or network configuration) from VAMeter.
- Modifying the core `smooth_ui_toolkit` library files.
