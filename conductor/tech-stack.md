# Technology Stack

## Core Technologies
- **Programming Language:** C++ (standard C++17 or later, as supported by ESP-IDF) and C for low-level drivers.
- **Base Framework:** [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/index.html) - The official development framework for Espressif SoCs.

## Graphics & UI
- **GUI Library:** [LVGL (Light and Versatile Graphics Library)](https://lvgl.io/) - Used for creating the user interface components, handling events, and animations.
- **Graphics Driver:** [LovyanGFX](https://github.com/lovyan03/LovyanGFX) - A high-performance graphics library used as the backend for display output and hardware abstraction.
- **Application Logic:** [Mooncake](https://github.com/m5stack/Mooncake) - A lightweight framework used to manage the lifecycle and interaction of multiple sub-applications.
- **Design Reference:** The [VAMeter-Firmware](./demo/VAMeter-Firmware) project serves as the primary reference for both the Mooncake implementation and the GUI design patterns.

## Hardware Abstraction
- **Peripherals:** 
    - PWM for Fan Speed Control.
    - I2C/SPI for Display Interface (depending on hardware configuration).
    - GPIO for button inputs and Hall effect sensor integration for the control wheel.
- **Custom Components:**
    - `app_button`: Specialized driver for handling physical button events.
    - `hall_wheel`: Driver for interpreting signals from Hall effect sensors used in the scroll wheel.

## Build & Deployment
- **Build System:** CMake (integrated with ESP-IDF `idf.py`).
- **Build Process:** **Manual.** The AI agent will perform code modifications and architectural tasks. The user is responsible for the actual compilation, flashing, and hardware verification.
