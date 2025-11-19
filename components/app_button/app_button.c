/**
 * @file app_button.c
 * @author Solonix-Chu
 * @brief 按键驱动的 C 语言实现
 * @version C-port
 * @date 2025-11-06
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <stdio.h>
#include <stdint.h>     // 替代 <cstdint>
#include <stdbool.h>    // 引入 'bool' 类型
#include <esp_log.h>
#include <esp_timer.h>
#include "app_button.h" // 对应的 C 语言头文件
#include "hall_wheel.h"
#include "iot_button.h" // 显式包含 iot_button
#include "button_gpio.h"

static const char* TAG = "app_button";

#define CONFIG_BUTTON_UP_GPIO_NUM GPIO_NUM_3
#define CONFIG_BUTTON_DOWN_GPIO_NUM GPIO_NUM_4
#define CONFIG_BUTTON_LEFT_GPIO_NUM GPIO_NUM_7
#define CONFIG_BUTTON_RIGHT_GPIO_NUM GPIO_NUM_10
#define CONFIG_BUTTON_OK_GPIO_NUM GPIO_NUM_2
#define CONFIG_BUTTON_POWER_GPIO_NUM GPIO_NUM_2
#define CONFIG_BUTTON_ACTIVE_LEVEL 0


// --- 用于每个按键的全局静态实例 ---
// 这些是实际的变量存储位置
static app_button_t g_btn_up;
static app_button_t g_btn_down;
static app_button_t g_btn_left;
static app_button_t g_btn_right;
static app_button_t g_btn_ok;
static app_button_t g_btn_power;

// --- 公开的全局指针 ---
// 这些指针在 app_button_init 中指向对应的静态实例，供外部模块访问
app_button_t* BtnUp = NULL;
app_button_t* BtnDown = NULL;
app_button_t* BtnLeft = NULL;
app_button_t* BtnRight = NULL;
app_button_t* BtnOk = NULL;
app_button_t* BtnPower = NULL;


// --- 内部实现函数 (替代 C++ 方法) ---

/**
 * @brief 设置按键状态 (公开函数的实现)
 */
void app_button_set_state(app_button_t* btn, uint32_t msec, app_button_state_t state) {
    if (!btn) return;
    btn->currentState = state;
    btn->lastMsec = msec;
}

/**
 * @brief 设置原始状态 (替代 setRawState - 保留原始注释)
 */
static void app_button_set_raw_state(app_button_t* btn, uint32_t msec, bool press) {
    // if (btn->_raw_press == press) {
    //     return;
    // }
    
    // btn->_raw_press = press;
    // btn->_lastRawChange = msec;
    
    // // 记录旧状态
    // btn->_oldPress = btn->_press;
    
    // // 设置新状态
    // if (press) {
    //     btn->_press = 1;  // 设为按下状态
    //     btn->_lastChange = msec;
    // } else {
    //     // 记录按住时间
    //     btn->_lastHoldPeriod = msec - btn->_lastChange;
        
    //     // 否则是普通点击
    //     btn->_press = 0;
    // }
}

/**
 * @brief 更新单个按键实例的内部状态 (替代 ButtonEspIdf_Class::update)
 */
static void app_button_instance_update(app_button_t* btn, uint32_t current_time) {
    if (!btn) return;
    // 在每个循环中重置状态为“无变化”
    // btn->currentState = APP_BUTTON_STATE_NOCHANGE;
    btn->lastMsec = current_time;
}


// --- C 语言回调函数 (修改为使用结构体指针) ---

// // 按下回调 (原始代码中已注释)
// static void btn_press_down_cb(void *arg, void *usr_data) {
//     app_button_t* btn = (app_button_t*)usr_data;
//     uint32_t current_time = esp_timer_get_time() / 1000;
//     app_button_set_raw_state(btn, current_time, true);
//     ESP_LOGI(TAG, "按钮被按下");
//     if (btn->_pressed_cb) {
//         btn->_pressed_cb();
//     }
// }
// // 释放回调 (原始代码中已注释)
// static void btn_press_end_cb(void *arg, void *usr_data) {
//     app_button_t* btn = (app_button_t*)usr_data;
//     uint32_t current_time = esp_timer_get_time() / 1000;
//     app_button_set_raw_state(btn, current_time, false);
//     ESP_LOGI(TAG, "按钮被释放");
// }

// 按钮单击回调
static void btn_click_cb(void *arg, void *usr_data) {
    app_button_t* btn = (app_button_t*)usr_data; // 获取实例指针
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_CLICKED);
    ESP_LOGI(TAG, "按钮被点击");
}

// 按钮长按回调
static void btn_long_press_cb(void *arg, void *usr_data) {
    app_button_t* btn = (app_button_t*)usr_data; // 获取实例指针
    uint32_t current_time = esp_timer_get_time() / 1000;
    
    // 判断是否为OK按钮
    if (btn->_gpio_num == CONFIG_BUTTON_OK_GPIO_NUM) {
        ESP_LOGI(TAG, "OK按钮被长按");
        app_button_set_state(btn, current_time, APP_BUTTON_STATE_HOLD);
    } else {
        // 其他按钮的长按处理 (原始代码中已注释)
        // app_button_set_state(btn, current_time, APP_BUTTON_STATE_HOLD);
        // ESP_LOGI(TAG, "按钮被长按");
    }
}

// 按钮双击回调
static void btn_double_click_cb(void *arg, void *usr_data) {
    app_button_t* btn = (app_button_t*)usr_data; // 获取实例指针
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_DECIDE_CLICK_COUNT);
    ESP_LOGI(TAG, "按钮被双击");
}

// --- 霍尔滚轮回调函数 ---

static void hall_wheel_up_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data) {
    app_button_t* btn = (app_button_t*)user_data; // 获取对应的按键实例
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_CLICKED);
    ESP_LOGI(TAG, "上方向键触发");
}

static void hall_wheel_down_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data) {
    app_button_t* btn = (app_button_t*)user_data;
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_CLICKED);
    ESP_LOGI(TAG, "下方向键触发");
}

static void hall_wheel_left_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data) {
    app_button_t* btn = (app_button_t*)user_data;
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_CLICKED);
    ESP_LOGI(TAG, "左方向键触发");
}

static void hall_wheel_right_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data) {
    app_button_t* btn = (app_button_t*)user_data;
    uint32_t current_time = esp_timer_get_time() / 1000;
    app_button_set_state(btn, current_time, APP_BUTTON_STATE_CLICKED);
    ESP_LOGI(TAG, "右方向键触发");
}


// --- 初始化函数 (替代 C++ 方法) ---

/**
 * @brief 基础初始化
 * 用于不使用 iot_button 的按键 (即霍尔滚轮按键)
 */
static void app_button_init_base(app_button_t* btn, int gpio_num, uint8_t active_level) {
    if (gpio_num < 0) {
        ESP_LOGW(TAG, "按键 GPIO %d 已禁用, 跳过初始化", gpio_num);
        return;
    }
    btn->_gpio_num = gpio_num;
    btn->_pressed_cb = NULL;
    btn->_btn_handle = NULL;
}

/**
 * @brief 使用 iot_button 进行初始化
 * 用于 OK 和 Power 键
 */
static void app_button_init_iot(app_button_t* btn, int gpio_num, uint8_t active_level) {
    if (gpio_num < 0) {
        ESP_LOGW(TAG, "按键 GPIO %d 已禁用, 跳过初始化", gpio_num);
        return;
    }
    
    btn->_gpio_num = gpio_num;
    btn->_pressed_cb = NULL;
    btn->_btn_handle = NULL;
    
    // 按钮配置
    button_config_t btn_cfg = {
        .long_press_time = 1000,
        .short_press_time = 180
    };
    
    // GPIO 配置
    button_gpio_config_t gpio_cfg = {
        .gpio_num = gpio_num,
        .active_level = active_level,
        .enable_power_save = true,
        .disable_pull = false
    };
    
    // 创建 iot_button 实例
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn->_btn_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "创建按钮失败: %d, GPIO: %d", ret, gpio_num);
        return;
    }
    
    // 注册回调函数, 将 'btn' (指向此按键结构体的指针) 作为 user_data 传递
    // iot_button_register_cb(btn->_btn_handle, BUTTON_PRESS_DOWN, NULL, btn_press_down_cb, btn);
    // iot_button_register_cb(btn->_btn_handle, BUTTON_PRESS_END, NULL, btn_press_end_cb, btn);
    iot_button_register_cb(btn->_btn_handle, BUTTON_SINGLE_CLICK, NULL, btn_click_cb, btn);
    iot_button_register_cb(btn->_btn_handle, BUTTON_LONG_PRESS_START, NULL, btn_long_press_cb, btn);
    // iot_button_register_cb(btn->_btn_handle, BUTTON_DOUBLE_CLICK, NULL, btn_double_click_cb, btn);
}


// --- 公开函数 (全局 API) ---

/**
 * @brief 初始化所有按键 (替代 ButtonEspIdf::init)
 */
void app_button_init(void) {
    ESP_LOGI(TAG, "初始化按键...");
    
    // 从 Kconfig 获取 GPIO 配置
    const int GPIO_BUTTON_UP = CONFIG_BUTTON_UP_GPIO_NUM;
    const int GPIO_BUTTON_DOWN = CONFIG_BUTTON_DOWN_GPIO_NUM;
    const int GPIO_BUTTON_LEFT = CONFIG_BUTTON_LEFT_GPIO_NUM;
    const int GPIO_BUTTON_RIGHT = CONFIG_BUTTON_RIGHT_GPIO_NUM;
    const int GPIO_BUTTON_OK = CONFIG_BUTTON_OK_GPIO_NUM;
    const int GPIO_BUTTON_POWER = CONFIG_BUTTON_POWER_GPIO_NUM;
    const uint8_t BUTTON_ACTIVE_LEVEL = CONFIG_BUTTON_ACTIVE_LEVEL;
    
    // --- 初始化 OK 和 Power 键 ---
    // 这两个键使用 esp-iot-button 驱动
    app_button_init_iot(&g_btn_ok, GPIO_BUTTON_OK, BUTTON_ACTIVE_LEVEL);
    // app_button_init_iot(&g_btn_power, GPIO_BUTTON_POWER, BUTTON_ACTIVE_LEVEL);
    
    // --- 初始化方向键 ---
    // 这四个键使用自定义的 hall_wheel 驱动，因此只做基础结构体初始化
    app_button_init_base(&g_btn_up, GPIO_BUTTON_UP, BUTTON_ACTIVE_LEVEL);
    app_button_init_base(&g_btn_down, GPIO_BUTTON_DOWN, BUTTON_ACTIVE_LEVEL);
    app_button_init_base(&g_btn_left, GPIO_BUTTON_LEFT, BUTTON_ACTIVE_LEVEL);
    app_button_init_base(&g_btn_right, GPIO_BUTTON_RIGHT, BUTTON_ACTIVE_LEVEL);
    
    // --- 配置霍尔滚轮 ---
    hall_wheel_config_t wheel_config = {
        .gpio_num = (gpio_num_t)GPIO_BUTTON_UP, // 临时值, 下面会覆盖
        .active_level = BUTTON_ACTIVE_LEVEL,
        .wave_duration_ms = 2,
        .detection_window_ms = 100,
        .min_pulses = 2
    };
    
    // 创建 上 滚轮设备
    hall_wheel_handle_t wheel_up = NULL;
    wheel_config.gpio_num = (gpio_num_t)GPIO_BUTTON_UP;
    esp_err_t ret = hall_wheel_create(&wheel_config, &wheel_up);
    if (ret == ESP_OK) {
        // 注册回调, 将 'g_btn_up' 实例的地址作为 user_data 传递
        hall_wheel_register_cb(wheel_up, HALL_WHEEL_EVENT_CLICK, hall_wheel_up_cb, &g_btn_up);
    } else {
        ESP_LOGE(TAG, "创建上键霍尔滚轮失败: %d", ret);
    }
    
    // 创建 下 滚轮设备
    hall_wheel_handle_t wheel_down = NULL;
    wheel_config.gpio_num = (gpio_num_t)GPIO_BUTTON_DOWN;
    ret = hall_wheel_create(&wheel_config, &wheel_down);
    if (ret == ESP_OK) {
        hall_wheel_register_cb(wheel_down, HALL_WHEEL_EVENT_CLICK, hall_wheel_down_cb, &g_btn_down);
    } else {
        ESP_LOGE(TAG, "创建下键霍尔滚轮失败: %d", ret);
    }
    
    // 创建 左 滚轮设备
    hall_wheel_handle_t wheel_left = NULL;
    wheel_config.gpio_num = (gpio_num_t)GPIO_BUTTON_LEFT;
    ret = hall_wheel_create(&wheel_config, &wheel_left);
    if (ret == ESP_OK) {
        hall_wheel_register_cb(wheel_left, HALL_WHEEL_EVENT_CLICK, hall_wheel_left_cb, &g_btn_left);
    } else {
        ESP_LOGE(TAG, "创建左键霍尔滚轮失败: %d", ret);
    }
    
    // 创建 右 滚轮设备
    hall_wheel_handle_t wheel_right = NULL;
    wheel_config.gpio_num = (gpio_num_t)GPIO_BUTTON_RIGHT;
    ret = hall_wheel_create(&wheel_config, &wheel_right);
    if (ret == ESP_OK) {
        hall_wheel_register_cb(wheel_right, HALL_WHEEL_EVENT_CLICK, hall_wheel_right_cb, &g_btn_right);
    } else {
        ESP_LOGE(TAG, "创建右键霍尔滚轮失败: %d", ret);
    }
    
    // --- 设置公开的全局指针 ---
    // 将外部可见的指针指向内部的静态实例
    BtnUp = &g_btn_up;
    BtnDown = &g_btn_down;
    BtnLeft = &g_btn_left;
    BtnRight = &g_btn_right;
    BtnOk = &g_btn_ok;
    BtnPower = &g_btn_power;
    
    ESP_LOGI(TAG, "按键初始化完成");
}

/**
 * @brief 更新所有按键的状态 (替代 ButtonEspIdf::update)
 */
void app_button_update(void) {
    // 获取当前时间
    uint32_t current_time = esp_timer_get_time() / 1000;
    
    // 更新所有按键实例的状态
    // (在C版本中，这主要用于将 currentState 重置为 NOCHANGE)
    app_button_instance_update(&g_btn_power, current_time);
    app_button_instance_update(&g_btn_ok, current_time);
    app_button_instance_update(&g_btn_up, current_time);
    app_button_instance_update(&g_btn_down, current_time);
    app_button_instance_update(&g_btn_left, current_time);
    app_button_instance_update(&g_btn_right, current_time);
}