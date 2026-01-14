/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <esp_log.h>
#include <lvgl.h>
#include <LovyanGFX.hpp>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <esp_task_wdt.h> // Required for esp_task_wdt_add
#include <esp_timer.h>

#define LV_TICK_PERIOD_MS 20 

static const char* TAG = "hal_lvgl";

static lgfx::LGFX_Device* _disp_ptr = nullptr;
static SemaphoreHandle_t xGuiSemaphore = NULL;

static void _lv_tick_task(void *arg) {
    (void) arg;
    lv_tick_inc(1); // 告诉 LVGL 过去了 1ms
}

/* Flush the content of the internal buffer to the specific area on the display */
static void _disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    // Simple log to track flush count
    static int flush_count = 0;
    if (flush_count++ < 5) { // Log first 5 flushes only to avoid flood
        ESP_LOGI(TAG, "flush: %d,%d -> %d,%d", (int)area->x1, (int)area->y1, (int)area->x2, (int)area->y2);
    }
    
    if (!_disp_ptr)
    {
        lv_display_flush_ready(disp);
        return;
    }

    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    _disp_ptr->startWrite();
    _disp_ptr->setWindow(area->x1, area->y1, area->x2, area->y2);

    // Cast byte pointer to uint16_t pointer (assuming LV_COLOR_DEPTH=16)
    // swap=true to handle endianness if needed (common for LGFX+LVGL)
    _disp_ptr->pushPixels((uint16_t *)px_map, w * h, true);

    _disp_ptr->endWrite();

    /* IMPORTANT: Inform the graphics library that you are ready with the flushing */
    lv_display_flush_ready(disp);
}

static void _lv_port_disp_init()
{
    if (!_disp_ptr)
    {
        ESP_LOGE(TAG, "Display pointer is null");
        return;
    }

    /* 1. Create the display in LVGL */
    lv_display_t *disp = lv_display_create(_disp_ptr->width(), _disp_ptr->height());
    if (!disp)
    {
        ESP_LOGE(TAG, "Failed to create display");
        return;
    }

    /* 2. Set flush callback */
    lv_display_set_flush_cb(disp, _disp_flush);

    /* 3. Allocate draw buffer */
    // Use a small partial buffer to avoid large allocations
    uint32_t buf_lines = 8; // 8 rows of pixels
    size_t buf_size_bytes = _disp_ptr->width() * buf_lines * sizeof(lv_color_t);
    if (buf_size_bytes < 256) buf_size_bytes = 256;

    void *buf1 = heap_caps_malloc(buf_size_bytes, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!buf1)
    {
        ESP_LOGW(TAG, "DMA/Internal malloc failed, trying generic malloc");
        buf1 = malloc(buf_size_bytes);
    }

    if (!buf1)
    {
        ESP_LOGE(TAG, "LVGL buffer malloc failed (%d bytes)", (int)buf_size_bytes);
        return;
    }

    ESP_LOGI(TAG, "Display buffer allocated (partial): %d bytes", (int)buf_size_bytes);

    /* 4. Set buffer */
    // LV_DISPLAY_RENDER_MODE_PARTIAL is suitable for most cases
    lv_display_set_buffers(disp, buf1, nullptr, buf_size_bytes, LV_DISPLAY_RENDER_MODE_PARTIAL);
}

void HAL_PocketFan::lvgl_update()
{
    if (xGuiSemaphore)
    {
        // 使用非阻塞或短超时，防止死锁卡住主循环
        if (xSemaphoreTakeRecursive(xGuiSemaphore, 5 / portTICK_PERIOD_MS) == pdTRUE)
        {
            lv_timer_handler();
            xSemaphoreGiveRecursive(xGuiSemaphore);
        }
    }
    else
    {
        // 如果没有初始化锁，直接运行（单任务模式）
        lv_timer_handler();
    }
}

void HAL_PocketFan::_lvgl_init()
{
    _disp_ptr = HAL::GetDisplay();
    bool invert = HAL::GetSystemConfig().invertDisplay;
    _disp_ptr->invertDisplay(invert); // Apply configured invert state for SSD1306
    ESP_LOGI(TAG, "invertDisplay: %s", invert ? "ON" : "OFF");

    // Create the mutex for LVGL
    xGuiSemaphore = xSemaphoreCreateRecursiveMutex();
    if (!xGuiSemaphore) {
        ESP_LOGE(TAG, "Failed to create LVGL semaphore");
        return;
    }

    if (!lv_is_initialized())
    {
        lv_init();
    }

    _lv_port_disp_init();

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &_lv_tick_task,
        .name = "lvgl_tick_timer"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    // 启动定时器，周期为 1000 微秒 (1ms)
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 1000));

    ESP_LOGI(TAG, "LVGL tick timer started");
}

void HAL_PocketFan::lvgl_lock()
{
    if (xGuiSemaphore) {
        xSemaphoreTakeRecursive(xGuiSemaphore, portMAX_DELAY);
    }
}

void HAL_PocketFan::lvgl_unlock()
{
    if (xGuiSemaphore) {
        xSemaphoreGiveRecursive(xGuiSemaphore);
    }
}
