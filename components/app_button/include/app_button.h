/*
 * app_button.h
 * 按键驱动的 C 语言头文件
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "iot_button.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 按键的可能状态 (替换 C++ 枚举)
 */
typedef enum {
    APP_BUTTON_STATE_NOCHANGE = 0,
    APP_BUTTON_STATE_CLICKED,
    APP_BUTTON_STATE_DECIDE_CLICK_COUNT,
    APP_BUTTON_STATE_HOLD,
} app_button_state_t;

/**
 * @brief 按键结构体 (替换 C++ 的 ButtonEspIdf_Class 类)
 */
typedef struct {
    // 公开状态
    app_button_state_t currentState;
    uint32_t lastMsec;

    // "私有" 配置和句柄
    int _gpio_num;
    button_handle_t _btn_handle; // iot_button 句柄
    void (*_pressed_cb)(void);   // 保留以保持相似性，但未使用

    // 内部状态 (来自 setRawState)
    bool _raw_press;
    uint32_t _lastRawChange;
    bool _oldPress;
    bool _press;
    uint32_t _lastChange;
    uint32_t _lastHoldPeriod;
    
} app_button_t;

// --- 指向按钮实例的全局指针 ---
// 这些指针在 app_button.c 中被初始化，指向内部的静态实例
extern app_button_t* BtnUp;
extern app_button_t* BtnDown;
extern app_button_t* BtnLeft;
extern app_button_t* BtnRight;
extern app_button_t* BtnOk;
extern app_button_t* BtnPower;

/**
 * @brief 初始化按键管理器 (替换 ButtonEspIdf::init)
 */
void app_button_init(void);

/**
 * @brief 更新按键状态 (替换 ButtonEspIdf::update)
 * (在C实现中，这个函数主要用于重置状态，
 * 因为实际的状态更新是由 iot_button 和 hall_wheel 的回调函数驱动的)
 */
void app_button_update(void);

/**
 * @brief 设置按键状态 (替换 ButtonEspIdf_Class::setState)
 * 由回调函数调用。
 *
 * @param btn 指向按键结构体的指针
 * @param msec 当前时间戳 (毫秒)
 * @param state 新的按键状态
 */
void app_button_set_state(app_button_t* btn, uint32_t msec, app_button_state_t state);

#ifdef __cplusplus
}
#endif