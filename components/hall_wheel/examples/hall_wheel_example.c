/* 霍尔滚轮示例 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "hall_wheel.h"

static const char *TAG = "hall_wheel_example";

// 霍尔滚轮回调函数
static void hall_wheel_click_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data)
{
    const char *name = (const char *)user_data;
    ESP_LOGI(TAG, "霍尔滚轮 '%s' 点击事件触发", name);
}

void hall_wheel_example(void)
{
    ESP_LOGI(TAG, "霍尔滚轮示例开始");
    
    // 创建两个霍尔滚轮设备，分别连接到不同的GPIO
    hall_wheel_handle_t wheel1 = NULL;
    hall_wheel_handle_t wheel2 = NULL;
    
    // 第一个滚轮配置（使用GPIO 5）
    hall_wheel_config_t wheel1_config = {
        .gpio_num = GPIO_NUM_8,      // 使用GPIO 8
        .active_level = 0,           // 低电平有效
        .wave_duration_ms = 30,      // 方波持续时间30ms
        .detection_window_ms = 150,  // 检测窗口150ms
        .min_pulses = 2              // 最小脉冲数2
    };
    
    // 第二个滚轮配置（使用GPIO 6）
    hall_wheel_config_t wheel2_config = {
        .gpio_num = GPIO_NUM_9,      // 使用GPIO 9
        .active_level = 0,           // 低电平有效
        .wave_duration_ms = 40,      // 方波持续时间40ms
        .detection_window_ms = 200,  // 检测窗口200ms 
        .min_pulses = 3              // 最小脉冲数3
    };
    
    // 创建霍尔滚轮设备
    esp_err_t ret = hall_wheel_create(&wheel1_config, &wheel1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建霍尔滚轮1失败，错误码：%d", ret);
        return;
    }
    
    ret = hall_wheel_create(&wheel2_config, &wheel2);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建霍尔滚轮2失败，错误码：%d", ret);
        hall_wheel_delete(wheel1);
        return;
    }
    
    // 注册回调
    hall_wheel_register_cb(wheel1, HALL_WHEEL_EVENT_CLICK, hall_wheel_click_cb, "滚轮1");
    hall_wheel_register_cb(wheel2, HALL_WHEEL_EVENT_CLICK, hall_wheel_click_cb, "滚轮2");
    
    ESP_LOGI(TAG, "霍尔滚轮已初始化，等待事件...");
    ESP_LOGI(TAG, "全局定时器已启动，周期扫描所有霍尔滚轮设备");
    
    // 主循环，等待事件触发
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    // 注意：此处永远不会执行到，但在实际应用中应该在程序退出前清理资源
    hall_wheel_delete(wheel1);
    hall_wheel_delete(wheel2);
} 