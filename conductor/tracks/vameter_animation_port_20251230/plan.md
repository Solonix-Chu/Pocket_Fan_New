# Plan: VAMeter Animation Port & Animation Kit Component

## Phase 1: Setup & Infrastructure
- [x] Task: Create Component Structure
    - Create directory `components/va_effects`
    - Create `CMakeLists.txt` for the component
    - Create `idf_component.yml` (if needed for dependencies)
- [x] Task: Define Base Animation Factory API
    - Create `va_effects.h` and `va_effects.cpp`
    - Define the `VaEffect` static class structure
- [x] Task: Setup Test Application Scaffolding
    - Create `apps/animation_test_app` directory
    - Create `app.h`, `app.cpp` for the test app
    - Register the test app in the main system (if applicable)
- [ ] Task: Conductor - User Manual Verification 'Setup & Infrastructure' (Protocol in workflow.md)

## Phase 2: Core Animation Primitives (Page Transitions)
- [ ] Task: Implement Backlight Fade
    - Add `VaEffect::FadeBacklight(to_brightness, duration)`
    - TDD: Verify duration and value updates
- [ ] Task: Implement Circular Mask Reveal
    - Add `VaEffect::CircularReveal(center_x, center_y, duration)`
    - Port logic from `app_startup_anim`
- [ ] Task: Implement Slide Transitions
    - Add `VaEffect::SlideIn(direction, distance, duration)`
    - Add `VaEffect::SlideOut(direction, distance, duration)`
- [ ] Task: Update Test App (Gallery Mode - Basics)
    - Create "Transitions Gallery" page in test app
    - Add buttons to trigger each transition type
- [ ] Task: Conductor - User Manual Verification 'Core Animation Primitives' (Protocol in workflow.md)

## Phase 3: Menu & Widget Animations
- [ ] Task: Implement Staggered List Entrance
    - Add `VaEffect::StaggeredEntrance(items_list, delay_per_item)`
    - Port logic from `SelectMenuPage`
- [ ] Task: Implement Selector Animations
    - Add `VaEffect::SelectorMove(target_rect)`
    - Add `VaEffect::SelectorSqueeze()`
- [ ] Task: Implement Widget Animations
    - Add `VaEffect::Spinner(x, y, size)` (Continuous rotation)
    - Add `VaEffect::NotificationPop(message)` (Bubble expansion)
- [ ] Task: Update Test App (Menu & Widget Gallery)
    - Create "Menu Effects" page with a dummy list
    - Create "Widgets" page to toggle spinners/notifications
- [ ] Task: Conductor - User Manual Verification 'Menu & Widget Animations' (Protocol in workflow.md)

## Phase 4: Data Visualization Effects
- [ ] Task: Implement Chart Animation Wrappers
    - Add `VaEffect::ChartZoom(chart_ref, target_range)`
    - Add `VaEffect::ChartPan(chart_ref, offset)`
- [ ] Task: Implement Value Entrance
    - Add `VaEffect::ValueSlideIn(target_value, direction)`
- [ ] Task: Update Test App (Mock Apps)
    - Create "Mock Waveform" page to test chart animations
    - Create "Mock Power Monitor" page to test value entrance
- [ ] Task: Conductor - User Manual Verification 'Data Visualization Effects' (Protocol in workflow.md)

## Phase 5: Final Polish & Stress Testing
- [ ] Task: Implement Stress Test Mode
    - Add "Stress Test" page to Test App
    - Run multiple spinners and transitions simultaneously
- [ ] Task: Interactive Playground
    - Add "Playground" page to tweak duration/easing at runtime
- [ ] Task: Visual Tuning
    - Compare side-by-side (if possible) or visually verify easing curves match VAMeter
- [ ] Task: Documentation
    - Add `README.md` to `components/va_effects` documenting the API
- [ ] Task: Conductor - User Manual Verification 'Final Polish & Stress Testing' (Protocol in workflow.md)
