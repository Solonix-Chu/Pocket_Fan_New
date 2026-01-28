/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "hal.h"
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <cmath>
#include <sstream>
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// #include <mooncake.h> // Assuming not strictly needed for base HAL impl, but loop/update might use it

static const char* TAG = "HAL";

HAL* HAL::_hal = nullptr;

HAL* HAL::Get() { return _hal; }

bool HAL::Check() { return _hal != nullptr; }

bool HAL::Inject(HAL* hal)
{
    if (_hal != nullptr)
    {
        ESP_LOGE(TAG, "HAL already exist");
        return false;
    }

    if (hal == nullptr)
    {
        ESP_LOGE(TAG, "invalid HAL ptr");
        return false;
    }

    // Set _hal before init, as init might depend on HAL::Get()
    _hal = hal;

    hal->init();
    ESP_LOGI(TAG, "HAL injected, type: %s", hal->type().c_str());

    return true;
}

void HAL::Destroy()
{
    if (_hal == nullptr)
    {
        ESP_LOGE(TAG, "HAL not exist");
        return;
    }

    delete _hal;
    _hal = nullptr;
}

void HAL::delay(unsigned long milliseconds)
{
    vTaskDelay(pdMS_TO_TICKS(milliseconds));
}

unsigned long HAL::millis()
{
    return (unsigned long)(esp_timer_get_time() / 1000);
}

tm* HAL::getLocalTime()
{
    time(&_time_buffer);
    return localtime(&_time_buffer);
}

/* -------------------------------------------------------------------------- */
/*                                   Display                                  */
/* -------------------------------------------------------------------------- */

void HAL::renderFpsPanel()
{
    // Optional: Use lv_sysmon_show_fps(true) if supported and configured
}

void HAL::popFatalError(std::string msg)
{
    ESP_LOGE(TAG, "FATAL ERROR: %s", msg.c_str());
    reboot();
}

// void HAL::popFatalError(std::string msg)
// {
//     ESP_LOGE(TAG, "FATAL ERROR: %s", msg.c_str());
    
//     lv_obj_t * mbox = lv_msgbox_create(NULL);
//     lv_msgbox_add_title(mbox, "Fatal Error");
//     lv_msgbox_add_text(mbox, msg.c_str());
//     lv_msgbox_add_close_button(mbox);
    
//     // Wait loop for reboot
//     while (1) {
//         feedTheDog();
//         delay(50);
//         if (getAnyButton()) reboot();
//     }
// }

// void HAL::popWarning(std::string msg)
// {
//     ESP_LOGW(TAG, "WARNING: %s", msg.c_str());

//     lv_obj_t * mbox = lv_msgbox_create(NULL);
//     lv_msgbox_add_title(mbox, "Warning");
//     lv_msgbox_add_text(mbox, msg.c_str());
//     lv_msgbox_add_close_button(mbox);
    
//     // Wait loop? Original waited for button press to continue.
//     // LVGL msgbox is async (event based). 
//     // But to match original blocking behavior:
//     while (1) {
//         feedTheDog();
//         delay(50);
//         // lv_timer_handler(); // If we are in a loop, we might need to drive LVGL?
//         // Usually HAL is called from App Update.
//         // If we block here, LVGL won't update unless we call handler.
//         // But 'popWarning' implies a modal.
//         // For now, let's just log and show, and let the app continue if it's not a busy-wait loop.
//         // But the original was a busy-wait.
//         if (getAnyButton()) break;
//     }
//     // Close mbox?
//     lv_msgbox_close(mbox);
// }

// void HAL::popSuccess(std::string msg, bool showSuccessLabel)
// {
//     ESP_LOGI(TAG, "SUCCESS: %s", msg.c_str());

//     lv_obj_t * mbox = lv_msgbox_create(NULL);
//     lv_msgbox_add_title(mbox, showSuccessLabel ? "Success" : "Info");
//     lv_msgbox_add_text(mbox, msg.c_str());
//     lv_msgbox_add_close_button(mbox);

//     while (1) {
//         feedTheDog();
//         delay(50);
//         if (getAnyButton()) break;
//     }
//     lv_msgbox_close(mbox);
// }

void HAL::popWarning(std::string msg)
{
    ESP_LOGW(TAG, "WARNING: %s", msg.c_str());
    delay(100);
}

void HAL::popSuccess(std::string msg, bool showSuccessLabel)
{
    (void)showSuccessLabel;
    ESP_LOGI(TAG, "SUCCESS: %s", msg.c_str());
    delay(100);
}

/* -------------------------------------------------------------------------- */
/*                                Power Monitor                               */
/* -------------------------------------------------------------------------- */

POWER_MONITOR::UnitAdaptatedData_t HAL::GetUnitAdaptatedVoltage(const float& voltage)
{
    POWER_MONITOR::UnitAdaptatedData_t data;
    char buf[32];

    if (std::abs(voltage) >= 1.0)
    {
        snprintf(buf, sizeof(buf), "%.2f", voltage);
        data.unit = "V";
    }
    else if (std::abs(voltage) >= 0.001)
    {
        snprintf(buf, sizeof(buf), "%.1f", voltage * 1000.0);
        data.unit = "mV";
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.1f", voltage * 1000000.0);
        data.unit = "mV"; // "μV" not standard ascii, careful
    }
    data.value = std::string(buf);
    return data;
}

POWER_MONITOR::UnitAdaptatedData_t HAL::GetUnitAdaptatedCurrent(const float& current)
{
    POWER_MONITOR::UnitAdaptatedData_t data;
    char buf[32];

    if (std::abs(current) >= 1.0)
    {
        snprintf(buf, sizeof(buf), "%.3f", current);
        data.unit = "A";
    }
    else if (std::abs(current) >= 0.001)
    {
        snprintf(buf, sizeof(buf), "%.1f", current * 1000.0);
        data.unit = "mA";
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.1f", current * 1000000.0);
        data.unit = "uA"; 
    }
    data.value = std::string(buf);
    return data;
}

POWER_MONITOR::UnitAdaptatedData_t HAL::GetUnitAdaptatedPower(const float& power)
{
    POWER_MONITOR::UnitAdaptatedData_t data;
    char buf[32];

    if (std::abs(power) >= 1.0)
    {
        snprintf(buf, sizeof(buf), "%.2f", power);
        data.unit = "W";
    }
    else if (std::abs(power) >= 0.001)
    {
        snprintf(buf, sizeof(buf), "%.1f", power * 1000.0);
        data.unit = "mW";
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.0f", power * 1000000.0);
        data.unit = "uW";
    }
    data.value = std::string(buf);
    return data;
}

POWER_MONITOR::UnitAdaptatedData_t HAL::GetUnitAdaptatedCapacity(const float& capacity)
{
    POWER_MONITOR::UnitAdaptatedData_t data;
    char buf[32];

    if (std::abs(capacity) >= 1.0)
    {
        snprintf(buf, sizeof(buf), "%.3f", capacity);
        data.unit = "Ah";
    }
    else if (std::abs(capacity) >= 0.001)
    {
        snprintf(buf, sizeof(buf), "%.1f", capacity * 1000.0);
        data.unit = "mAh";
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.0f", capacity * 1000000.0);
        data.unit = "uAh";
    }
    data.value = std::string(buf);
    return data;
}

POWER_MONITOR::UnitAdaptatedData_t HAL::GetUnitAdaptatedEnergy(const float& energy)
{
    POWER_MONITOR::UnitAdaptatedData_t data;
    char buf[32];

    if (std::abs(energy) >= 1.0)
    {
        snprintf(buf, sizeof(buf), "%.3f", energy);
        data.unit = "Wh";
    }
    else if (std::abs(energy) >= 0.001)
    {
        snprintf(buf, sizeof(buf), "%.1f", energy * 1000.0);
        data.unit = "mWh";
    }
    else
    {
        snprintf(buf, sizeof(buf), "%.0f", energy * 1000000.0);
        data.unit = "uWh";
    }
    data.value = std::string(buf);
    return data;
}
