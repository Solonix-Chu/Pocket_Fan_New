/* 霍尔滚轮示例 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hall_wheel.h"

static const char *TAG = "hall_wheel_example";

// 统一的霍尔滚轮事件回调函数
static void hall_wheel_event_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data)
{
    const char *name = (const char *)user_data;
    uint32_t speed = 0;

    switch (event) {
        case HALL_WHEEL_EVENT_CLICK:
            hall_wheel_get_speed(handle, &speed);
            ESP_LOGI(TAG, "滚轮 '%s' 点击, 当前速度: %d PPS", name, speed);
            break;
        
        case HALL_WHEEL_EVENT_SCROLL_START:
            ESP_LOGI(TAG, "滚轮 '%s' 开始滚动", name);
            break;

        case HALL_WHEEL_EVENT_SCROLL_END:
            hall_wheel_get_speed(handle, &speed); // 速度此时应为0
            ESP_LOGI(TAG, "滚轮 '%s' 停止滚动, 最终速度: %d PPS", name, speed);
            break;

        default:
            break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "霍尔滚轮重构表示例开始");
    
    // 滚轮配置 (示例GPIO 8)
    // 针对高速滚动优化参数
    hall_wheel_config_t wheel_config = {
        .gpio_num = GPIO_NUM_8,      // 使用GPIO 8
        .active_level = 0,           // 低电平有效
        .wave_duration_ms = 2,       // 防抖时间2ms, 允许捕捉高速脉冲
        .detection_window_ms = 100,  // 100ms无脉冲则认为滚动结束
        .min_pulses = 3              // 至少3个脉冲才算一次有效滚动
    };
    
    hall_wheel_handle_t wheel_handle = NULL;
    esp_err_t ret = hall_wheel_create(&wheel_config, &wheel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建霍尔滚轮失败，错误码：%d", ret);
        return;
    }
    
    // 注册所有我们关心的事件
    hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_CLICK, hall_wheel_event_cb, "滚轮1");
    hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_SCROLL_START, hall_wheel_event_cb, "滚轮1");
    hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_SCROLL_END, hall_wheel_event_cb, "滚轮1");
    
    ESP_LOGI(TAG, "霍尔滚轮已初始化，等待事件...");
    
    // 主循环中可以做其他事情，例如周期性地获取速度
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        
        uint32_t current_speed = 0;
        if (hall_wheel_get_speed(wheel_handle, &current_speed) == ESP_OK) {
            ESP_LOGI(TAG, "主循环查询: 当前滚动速度为 %d PPS", current_speed);
        }
    }
    
    // 在实际应用中应该在程序退出前清理资源
    // hall_wheel_delete(wheel_handle);
}