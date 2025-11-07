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
    HALL_WHEEL_EVENT_CLICK = 0,  /* 点击事件（滚动触发） */
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
 */
typedef struct {
    gpio_num_t gpio_num;          /* GPIO 引脚号 */
    uint8_t active_level;          /* 有效电平 (通常为低电平0) */
    uint16_t wave_duration_ms;     /* 单个方波持续时间，默认30ms */
    uint16_t detection_window_ms;  /* 检测窗口时间，默认150ms */
    uint8_t min_pulses;            /* 最小脉冲数量，触发点击事件，默认2 */
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

#ifdef __cplusplus
}
#endif
