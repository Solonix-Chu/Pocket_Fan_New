# Initial Concept
The user wants to build a reference implementation for ESP32-based UI and hardware control, specifically for a pocket fan device.

# Product Guide

## Overview
This project is a reference implementation designed for the ESP32 platform, showcasing advanced integration of graphical user interfaces (GUI) and hardware control. The primary application is a "Pocket Fan" device, providing a polished and functional software stack for embedded developers and hardware designers.

## Core Features
- **Interactive GUI:** A modern and minimalist user interface built with LVGL and LovyanGFX, featuring smooth animations and transitions.
- **Dual-Page Dashboard:** A primary homepage displaying real-time telemetry data (Voltage, Current, Power, Temperature, Capacity) with scrollable pages and fluid transitions.
- **Integrated Fan Control:** Intuitive PWM (fan speed) adjustment (0-100%) directly from the main dashboard using an adaptive scroll speed algorithm.
- **System Settings:** Integrated menu for managing device parameters such as display brightness and language preferences.
- **Battery Management:** Real-time battery status monitoring to ensure reliable portable operation.
- **Hardware Diagnostics:** A dedicated hardware testing mode for verifying sensor data and actuator performance.
- **Multilingual Support:** A localization-ready architecture supporting multiple languages, including English and Chinese.

## Goals
- **Reference Implementation:** Serve as a high-quality example of how to combine complex UI frameworks (LVGL, LovyanGFX) with hardware peripherals on the ESP-IDF framework.
- **User-Centric Design:** Deliver a user experience that is intuitive and visually appealing for the end-users of the pocket fan.
- **Extensibility:** Maintain a modular structure (using the Mooncake framework) that allows for easy addition of new sub-applications or hardware features.

## Target Audience
- **End-users:** Consumers using the pocket fan device who expect a reliable and attractive interface.
- **Embedded Developers:** Professionals and hobbyists seeking a robust template for ESP32 UI development.

## Design Aesthetic
- **Minimalist & Modern:** The interface prioritizes clarity and elegance, utilizing clean lines, subtle animations, and a focused color palette to provide a "premium" feel on a small-scale display.
