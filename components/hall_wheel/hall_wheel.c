#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "hall_wheel.h"
#include "sdkconfig.h"

static const char *TAG = "hall_wheel";

// 宏定义
#define HALL_WHEEL_CHECK(a, str, ret) if(!(a)) { \
        ESP_LOGE(TAG, "%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str); \
        return (ret); \
    }

// 配置参数
#define HALL_WHEEL_MAX_CB        3  // 每个事件最多回调函数数量
#define DEFAULT_WAVE_DURATION    30  // 默认方波持续时间（毫秒）
#define DEFAULT_DETECTION_WINDOW 150 // 默认检测窗口时间（毫秒）
#define DEFAULT_MIN_PULSES       2   // 默认最小脉冲数量
#define POLL_INTERVAL_MS         5   // 定时器扫描间隔（毫秒）

/**
 * @brief 霍尔滚轮脉冲计数器状态
 */
typedef struct {
    uint8_t pulse_count;              // 当前周期内的脉冲计数
    int64_t last_change_time;         // 最后一次电平变化的时间戳
    bool last_level;                  // 上一次的电平状态
    int64_t detection_start_time;     // 检测开始时间戳
    bool detection_in_progress;       // 是否正在进行检测
} hall_wheel_pulse_state_t;

/**
 * @brief 回调函数节点
 */
typedef struct callback_node {
    hall_wheel_cb_t cb;               // 回调函数
    void *user_data;                  // 用户数据
    struct callback_node *next;       // 下一个节点
} callback_node_t;

/**
 * @brief 霍尔滚轮设备结构体
 */
typedef struct hall_wheel_dev {
    hall_wheel_config_t config;       // 配置
    hall_wheel_pulse_state_t state;   // 状态
    callback_node_t *callbacks[HALL_WHEEL_EVENT_MAX]; // 回调函数链表
    size_t cb_size[HALL_WHEEL_EVENT_MAX]; // 各事件的回调函数数量
    bool is_running;                  // 是否正在运行
    struct hall_wheel_dev *next;      // 下一个设备
} hall_wheel_dev_t;

// 全局变量
static hall_wheel_dev_t *g_hall_wheel_list = NULL;  // 设备链表头
static esp_timer_handle_t g_timer_handle = NULL;    // 全局定时器句柄
static bool g_is_timer_running = false;             // 定时器是否运行中

// 前向声明
static void hall_wheel_timer_cb(void *arg);
static void process_hall_wheel(hall_wheel_dev_t *wheel);
static void check_detection_window(hall_wheel_dev_t *wheel);

esp_err_t hall_wheel_create(const hall_wheel_config_t *config, hall_wheel_handle_t *handle_out)
{
    HALL_WHEEL_CHECK(config != NULL, "config is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(handle_out != NULL, "handle_out is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(GPIO_IS_VALID_GPIO(config->gpio_num), "Invalid GPIO number", ESP_ERR_INVALID_ARG);
    
    // 检查该GPIO是否已被使用
    hall_wheel_dev_t *temp = g_hall_wheel_list;
    while (temp) {
        if (temp->config.gpio_num == config->gpio_num) {
            ESP_LOGE(TAG, "GPIO %d already used by another hall wheel", config->gpio_num);
            return ESP_ERR_INVALID_STATE;
        }
        temp = temp->next;
    }
    
    ESP_LOGI(TAG, "Creating Hall wheel device on GPIO %d", config->gpio_num);
    
    // 分配内存
    hall_wheel_dev_t *wheel = calloc(1, sizeof(hall_wheel_dev_t));
    HALL_WHEEL_CHECK(wheel != NULL, "calloc wheel failed", ESP_ERR_NO_MEM);
    
    // 设置配置参数
    memcpy(&wheel->config, config, sizeof(hall_wheel_config_t));
    
    // 使用默认值，如果没有设置
    if (wheel->config.wave_duration_ms == 0) {
        wheel->config.wave_duration_ms = DEFAULT_WAVE_DURATION;
    }
    if (wheel->config.detection_window_ms == 0) {
        wheel->config.detection_window_ms = DEFAULT_DETECTION_WINDOW;
    }
    if (wheel->config.min_pulses == 0) {
        wheel->config.min_pulses = DEFAULT_MIN_PULSES;
    }
    
    // 配置GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << wheel->config.gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (wheel->config.active_level == 0) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (wheel->config.active_level == 1) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,  // 不使用GPIO中断，通过定时器轮询
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO %d, err=%d", wheel->config.gpio_num, ret);
        free(wheel);
        return ret;
    }
    
    // 初始化状态
    wheel->is_running = true;
    wheel->state.last_level = (gpio_get_level(wheel->config.gpio_num) == wheel->config.active_level);
    
    // 将设备添加到链表
    wheel->next = g_hall_wheel_list;
    g_hall_wheel_list = wheel;
    
    // 如果是第一个设备，创建全局定时器
    if (!g_timer_handle) {
        esp_timer_create_args_t timer_args = {
            .callback = hall_wheel_timer_cb,
            .arg = NULL,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "hall_wheel_timer"
        };
        ret = esp_timer_create(&timer_args, &g_timer_handle);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create timer, err=%d", ret);
            // 从链表移除
            g_hall_wheel_list = wheel->next;
            free(wheel);
            return ret;
        }
    }
    
    // 如果定时器未运行，启动定时器
    if (!g_is_timer_running) {
        esp_timer_start_periodic(g_timer_handle, POLL_INTERVAL_MS * 1000);
        g_is_timer_running = true;
    }
    
    *handle_out = wheel;
    return ESP_OK;
}

esp_err_t hall_wheel_register_cb(hall_wheel_handle_t handle, hall_wheel_event_t event, 
                               hall_wheel_cb_t cb, void *user_data)
{
    HALL_WHEEL_CHECK(handle != NULL, "handle is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(event < HALL_WHEEL_EVENT_MAX, "event out of range", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(cb != NULL, "callback is null", ESP_ERR_INVALID_ARG);
    
    hall_wheel_dev_t *wheel = (hall_wheel_dev_t *)handle;
    
    // 创建新的回调节点
    callback_node_t *new_node = calloc(1, sizeof(callback_node_t));
    HALL_WHEEL_CHECK(new_node != NULL, "calloc callback node failed", ESP_ERR_NO_MEM);
    
    new_node->cb = cb;
    new_node->user_data = user_data;
    new_node->next = NULL;
    
    // 添加到回调链表
    if (wheel->callbacks[event] == NULL) {
        wheel->callbacks[event] = new_node;
    } else {
        // 找到链表的最后一个节点
        callback_node_t *curr = wheel->callbacks[event];
        int count = 1;
        
        while (curr->next != NULL) {
            curr = curr->next;
            count++;
        }
        
        // 检查是否超过最大回调数
        if (count >= HALL_WHEEL_MAX_CB) {
            free(new_node);
            return ESP_ERR_NO_MEM;
        }
        
        curr->next = new_node;
    }
    
    wheel->cb_size[event]++;
    return ESP_OK;
}

esp_err_t hall_wheel_delete(hall_wheel_handle_t handle)
{
    HALL_WHEEL_CHECK(handle != NULL, "handle is null", ESP_ERR_INVALID_ARG);
    
    hall_wheel_dev_t *wheel = (hall_wheel_dev_t *)handle;
    
    // 从链表中移除设备
    hall_wheel_dev_t **curr;
    for (curr = &g_hall_wheel_list; *curr;) {
        hall_wheel_dev_t *entry = *curr;
        if (entry == wheel) {
            *curr = entry->next;
            break;
        } else {
            curr = &entry->next;
        }
    }
    
    // 释放所有回调函数
    for (int i = 0; i < HALL_WHEEL_EVENT_MAX; i++) {
        callback_node_t *node = wheel->callbacks[i];
        while (node != NULL) {
            callback_node_t *next = node->next;
            free(node);
            node = next;
        }
    }
    
    // 释放设备内存
    free(wheel);
    
    // 如果没有设备了，停止并删除定时器
    if (g_hall_wheel_list == NULL && g_is_timer_running) {
        esp_timer_stop(g_timer_handle);
        esp_timer_delete(g_timer_handle);
        g_timer_handle = NULL;
        g_is_timer_running = false;
    }
    
    return ESP_OK;
}

// 全局定时器回调函数，扫描所有设备
static void hall_wheel_timer_cb(void *arg)
{
    hall_wheel_dev_t *wheel = g_hall_wheel_list;
    while (wheel) {
        if (wheel->is_running) {
            process_hall_wheel(wheel);
        }
        wheel = wheel->next;
    }
}

// 处理单个霍尔滚轮设备
static void process_hall_wheel(hall_wheel_dev_t *wheel)
{
    // 获取当前电平和时间
    bool current_level = (gpio_get_level(wheel->config.gpio_num) == wheel->config.active_level);
    int64_t now = esp_timer_get_time() / 1000; // 转换为毫秒
    
    // 如果有正在进行的检测窗口，检查是否超时
    if (wheel->state.detection_in_progress) {
        int64_t elapsed = now - wheel->state.detection_start_time;
        if (elapsed >= wheel->config.detection_window_ms) {
            check_detection_window(wheel);
        }
    }
    
    // 检测电平变化
    if (wheel->state.last_level != current_level) {
        // 如果尚未开始检测，则开始新检测周期
        if (!wheel->state.detection_in_progress) {
            wheel->state.detection_in_progress = true;
            wheel->state.pulse_count = 0;
            wheel->state.detection_start_time = now;
        }
        
        // 计算两次电平变化的时间间隔
        int64_t interval = now - wheel->state.last_change_time;
        
        // 只有当时间间隔在合理范围内，才增加脉冲计数
        if (wheel->state.last_change_time == 0 || 
            interval >= wheel->config.wave_duration_ms / 2) { // 允许一定误差
            
            wheel->state.pulse_count++;
            ESP_LOGD(TAG, "Level change detected, pulse count: %d", wheel->state.pulse_count);
        }
        
        wheel->state.last_level = current_level;
        wheel->state.last_change_time = now;
    }
}

// 检查检测窗口是否满足触发条件
static void check_detection_window(hall_wheel_dev_t *wheel)
{
    // 检查脉冲数量是否满足触发条件
    if (wheel->state.pulse_count >= wheel->config.min_pulses) {
        ESP_LOGD(TAG, "Click event triggered with %d pulses", wheel->state.pulse_count);
        
        // 触发点击事件回调
        callback_node_t *node = wheel->callbacks[HALL_WHEEL_EVENT_CLICK];
        while (node != NULL) {
            node->cb(wheel, HALL_WHEEL_EVENT_CLICK, node->user_data);
            node = node->next;
        }
    }
    
    // 重置状态
    wheel->state.detection_in_progress = false;
    wheel->state.pulse_count = 0;
    wheel->state.last_change_time = 0;
}
