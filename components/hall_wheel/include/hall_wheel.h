/*
 * 霍尔滚轮按键驱动
 */

#pragma once

#include "esp_err.h"
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 霍尔滚轮设备句柄
 */
typedef void* hall_wheel_handle_t;

/**
 * @brief 霍尔滚轮事件类型
 */
typedef enum {
    HALL_WHEEL_EVENT_CLICK = 0,  /* 滚动产生的点击事件 */
    HALL_WHEEL_EVENT_SCROLL_START, /* 滚动开始事件 */
    HALL_WHEEL_EVENT_SCROLL_END,   /* 滚动结束事件 */
    HALL_WHEEL_EVENT_MAX         /* 事件数量 */
} hall_wheel_event_t;

/**
 * @brief 霍尔滚轮回调函数
 * 
 * @param handle 霍尔滚轮句柄
 * @param event 触发的事件类型
 * @param user_data 用户数据
 */
typedef void (*hall_wheel_cb_t)(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data);

/**
 * @brief 霍尔滚轮配置
 *
 * @note New implementation details:
 * - The driver is now interrupt-based to handle fast scrolling.
 * - `wave_duration_ms`: Interpreted as the minimum interval between edges for debouncing, in milliseconds. 
 *   For fast scrolling (e.g., 2.5ms pulse width), this should be set to a low value like 1 or 2. Default: 2ms.
 * - `detection_window_ms`: Interpreted as the scroll session timeout in ms. If no pulse occurs for this duration,
 *   the scroll session is considered ended. Default: 100ms.
 * - `min_pulses`: Minimum number of pulses to start a scroll session. This helps filter out accidental touches. Default: 2.
 */
typedef struct {
    gpio_num_t gpio_num;          /* GPIO 引脚号 */
    uint8_t active_level;          /* 有效电平 (通常为低电平0) */
    uint16_t wave_duration_ms;     /* (Debounce) 最小脉冲间隔, 单位ms, 用于过滤噪声. 默认 2ms. */
    uint16_t detection_window_ms;  /* (Timeout) 滚动会话超时, 单位ms. 默认 100ms. */
    uint8_t min_pulses;            /* (Threshold) 最小脉冲数, 用于启动滚动会话. 默认 2. */
} hall_wheel_config_t;

/**
 * @brief 创建霍尔滚轮设备
 * 
 * @param config 霍尔滚轮配置
 * @param handle_out 输出的霍尔滚轮句柄
 * @return ESP_OK 成功，其他值为错误码
 */
esp_err_t hall_wheel_create(const hall_wheel_config_t *config, hall_wheel_handle_t *handle_out);

/**
 * @brief 注册霍尔滚轮事件回调函数
 * 
 * @param handle 霍尔滚轮句柄
 * @param event 事件类型
 * @param cb 回调函数
 * @param user_data 用户数据
 * @return ESP_OK 成功，其他值为错误码
 */
esp_err_t hall_wheel_register_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, 
                               hall_wheel_cb_t cb, void *user_data);

/**
 * @brief 删除霍尔滚轮设备
 * 
 * @param handle 霍尔滚轮句柄
 * @return ESP_OK 成功，其他值为错误码
 */
esp_err_t hall_wheel_delete(hall_wheel_handle_t handle);

/**
 * @brief 获取当前滚动速度
 * 
 * @param handle 霍尔滚轮句柄
 * @param speed_pps 输出的速度，单位：脉冲/秒 (pulses per second)
 * @return ESP_OK 成功，其他值为错误码
 */
esp_err_t hall_wheel_get_speed(hall_wheel_handle_t handle, uint32_t *speed_pps);


#ifdef __cplusplus
}
#endif