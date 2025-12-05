#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
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
#define DEFAULT_DEBOUNCE_MS      2   // 默认防抖时间 (ms)
#define DEFAULT_TIMEOUT_MS       100 // 默认滚动会话超时 (ms)
#define DEFAULT_MIN_PULSES       2   // 默认最小脉冲数量
#define SPEED_AVG_COUNT          4   // 用于计算平均速度的脉冲数

// 中断处理和任务相关
#define ISR_QUEUE_LEN            16
#define TASK_PRIORITY            (configMAX_PRIORITIES - 5)
#define TASK_STACK_SIZE          (4096)

/**
 * @brief 滚动状态
 */
typedef enum {
    WHEEL_STATE_IDLE,
    WHEEL_STATE_SCROLLING,
} wheel_state_t;

/**
 * @brief 脉冲事件 (ISR -> Task)
 */
typedef struct {
    gpio_num_t gpio_num;
    int64_t timestamp;
} hall_pulse_event_t;

/**
 * @brief 霍尔滚轮内部状态
 */
typedef struct {
    wheel_state_t state;
    uint32_t pulse_count;
    int64_t last_pulse_time;
    uint32_t speed_pps;
    int64_t pulse_intervals[SPEED_AVG_COUNT];
    uint8_t interval_index;
} hall_wheel_internal_state_t;

/**
 * @brief 回调函数节点
 */
typedef struct callback_node {
    hall_wheel_cb_t cb;
    void *user_data;
    struct callback_node *next;
} callback_node_t;

/**
 * @brief 霍尔滚轮设备结构体
 */
typedef struct hall_wheel_dev {
    hall_wheel_config_t config;
    hall_wheel_internal_state_t state;
    callback_node_t *callbacks[HALL_WHEEL_EVENT_MAX];
    esp_timer_handle_t timeout_timer;
    struct hall_wheel_dev *next;
} hall_wheel_dev_t;

// 全局变量
static hall_wheel_dev_t *g_hall_wheel_list = NULL;
static QueueHandle_t g_pulse_evt_queue = NULL;
static TaskHandle_t g_task_handle = NULL;
static portMUX_TYPE g_list_mux = portMUX_INITIALIZER_UNLOCKED;

// 前向声明
static void hall_wheel_task(void *arg);
static void IRAM_ATTR gpio_isr_handler(void *arg);
static void scroll_timeout_cb(void *arg);

// 触发回调
static void trigger_callbacks(hall_wheel_dev_t *wheel, hall_wheel_event_t event)
{
    callback_node_t *node = wheel->callbacks[event];
    while (node) {
        node->cb(wheel, event, node->user_data);
        node = node->next;
    }
}

// 核心处理逻辑
static void process_pulse(hall_wheel_dev_t *wheel, int64_t timestamp)
{
    int64_t interval_us = timestamp - wheel->state.last_pulse_time;

    // 1. 防抖
    if (interval_us < wheel->config.wave_duration_ms * 1000) {
        return;
    }

    // 重置超时定时器
    esp_timer_stop(wheel->timeout_timer);
    esp_timer_start_once(wheel->timeout_timer, wheel->config.detection_window_ms * 1000);

    wheel->state.last_pulse_time = timestamp;
    wheel->state.pulse_count++;

    // 2. 状态机
    if (wheel->state.state == WHEEL_STATE_IDLE) {
        if (wheel->state.pulse_count >= wheel->config.min_pulses) {
            wheel->state.state = WHEEL_STATE_SCROLLING;
            trigger_callbacks(wheel, HALL_WHEEL_EVENT_SCROLL_START);
            trigger_callbacks(wheel, HALL_WHEEL_EVENT_CLICK);
        }
    } else { // WHEEL_STATE_SCROLLING
        trigger_callbacks(wheel, HALL_WHEEL_EVENT_CLICK);
    }

    // 3. 速度计算
    if (wheel->state.state == WHEEL_STATE_SCROLLING) {
        wheel->state.pulse_intervals[wheel->state.interval_index] = interval_us;
        wheel->state.interval_index = (wheel->state.interval_index + 1) % SPEED_AVG_COUNT;

        int64_t total_interval = 0;
        uint8_t count = 0;
        for (int i = 0; i < SPEED_AVG_COUNT; i++) {
            if (wheel->state.pulse_intervals[i] > 0) {
                total_interval += wheel->state.pulse_intervals[i];
                count++;
            }
        }
        if (count > 0) {
            int64_t avg_interval = total_interval / count;
            if (avg_interval > 0) {
                wheel->state.speed_pps = 1000000 / avg_interval;
            }
        }
    }
}

// 滚动超时回调
static void scroll_timeout_cb(void *arg)
{
    hall_wheel_dev_t *wheel = (hall_wheel_dev_t *)arg;
    if (wheel->state.state == WHEEL_STATE_SCROLLING) {
        trigger_callbacks(wheel, HALL_WHEEL_EVENT_SCROLL_END);
    }
    // 重置状态
    wheel->state.state = WHEEL_STATE_IDLE;
    wheel->state.pulse_count = 0;
    wheel->state.speed_pps = 0;
    memset(wheel->state.pulse_intervals, 0, sizeof(wheel->state.pulse_intervals));
    wheel->state.interval_index = 0;
}

// 全局处理任务
static void hall_wheel_task(void *arg)
{
    hall_pulse_event_t pulse_event;
    while (1) {
        if (xQueueReceive(g_pulse_evt_queue, &pulse_event, portMAX_DELAY)) {
            hall_wheel_dev_t *wheel = NULL;

            // Find the device handle in a critical section
            portENTER_CRITICAL(&g_list_mux);
            hall_wheel_dev_t *current = g_hall_wheel_list;
            while (current) {
                if (current->config.gpio_num == pulse_event.gpio_num) {
                    wheel = current;
                    break;
                }
                current = current->next;
            }
            portEXIT_CRITICAL(&g_list_mux);

            // Process the pulse outside the critical section
            if (wheel) {
                process_pulse(wheel, pulse_event.timestamp);
            }
        }
    }
}

// ISR handler
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
    gpio_num_t gpio_num = (gpio_num_t)arg;
    hall_pulse_event_t event = {
        .gpio_num = gpio_num,
        .timestamp = esp_timer_get_time(),
    };
    xQueueSendFromISR(g_pulse_evt_queue, &event, NULL);
}

// 全局资源初始化
static esp_err_t hall_wheel_init_global(void)
{
    if (g_pulse_evt_queue == NULL) {
        g_pulse_evt_queue = xQueueCreate(ISR_QUEUE_LEN, sizeof(hall_pulse_event_t));
        HALL_WHEEL_CHECK(g_pulse_evt_queue, "create queue failed", ESP_ERR_NO_MEM);
    }

    if (g_task_handle == NULL) {
        BaseType_t res = xTaskCreate(hall_wheel_task, "hall_wheel_task", TASK_STACK_SIZE, NULL, TASK_PRIORITY, &g_task_handle);
        HALL_WHEEL_CHECK(res == pdPASS, "create task failed", ESP_ERR_NO_MEM);
    }
    
    // esp_err_t ret = gpio_install_isr_service(0); // ESP_INTR_FLAG_LEVEL1
    // if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    //     ESP_LOGE(TAG, "install isr service failed: %s", esp_err_to_name(ret));
    //     return ret;
    // }

    return ESP_OK;
}

// 全局资源销毁
static void hall_wheel_deinit_global(void)
{
    if (g_task_handle) {
        vTaskDelete(g_task_handle);
        g_task_handle = NULL;
    }
    if (g_pulse_evt_queue) {
        vQueueDelete(g_pulse_evt_queue);
        g_pulse_evt_queue = NULL;
    }
    gpio_uninstall_isr_service();
}

esp_err_t hall_wheel_create(const hall_wheel_config_t *config, hall_wheel_handle_t *handle_out)
{
    static int inited_flag; 
    esp_err_t ret;
    HALL_WHEEL_CHECK(config != NULL, "config is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(handle_out != NULL, "handle_out is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(GPIO_IS_VALID_GPIO(config->gpio_num), "Invalid GPIO number", ESP_ERR_INVALID_ARG);

    portENTER_CRITICAL(&g_list_mux);
    // 检查GPIO是否已被使用
    hall_wheel_dev_t *temp = g_hall_wheel_list;
    while (temp) {
        if (temp->config.gpio_num == config->gpio_num) {
            portEXIT_CRITICAL(&g_list_mux);
            ESP_LOGE(TAG, "GPIO %d already used", config->gpio_num);
            return ESP_ERR_INVALID_STATE;
        }
        temp = temp->next;
    }
    portEXIT_CRITICAL(&g_list_mux);

    if(!inited_flag)
    {
        // 初始化全局资源
        ret = hall_wheel_init_global();
        if (ret != ESP_OK) return ret;
        inited_flag = 1;
    }
    

    // 分配设备内存
    hall_wheel_dev_t *wheel = calloc(1, sizeof(hall_wheel_dev_t));
    HALL_WHEEL_CHECK(wheel != NULL, "calloc wheel failed", ESP_ERR_NO_MEM);

    // 设置配置，应用默认值
    wheel->config = *config;
    if (wheel->config.wave_duration_ms == 0) wheel->config.wave_duration_ms = DEFAULT_DEBOUNCE_MS;
    if (wheel->config.detection_window_ms == 0) wheel->config.detection_window_ms = DEFAULT_TIMEOUT_MS;
    if (wheel->config.min_pulses == 0) wheel->config.min_pulses = DEFAULT_MIN_PULSES;

    // 创建超时定时器
    esp_timer_create_args_t timer_args = {
        .callback = scroll_timeout_cb,
        .arg = wheel,
        .name = "hall_wheel_timeout"
    };
    ret = esp_timer_create(&timer_args, &wheel->timeout_timer);
    if (ret != ESP_OK) {
        free(wheel);
        return ret;
    }

    // 配置GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << wheel->config.gpio_num),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = (wheel->config.active_level == 0) ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
        .pull_down_en = (wheel->config.active_level == 1) ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        esp_timer_delete(wheel->timeout_timer);
        free(wheel);
        return ret;
    }
    
    // 添加ISR处理
    ret = gpio_isr_handler_add(wheel->config.gpio_num, gpio_isr_handler, (void *)wheel->config.gpio_num);
    if (ret != ESP_OK) {
        esp_timer_delete(wheel->timeout_timer);
        free(wheel);
        return ret;
    }

    // 添加到设备链表
    portENTER_CRITICAL(&g_list_mux);
    wheel->next = g_hall_wheel_list;
    g_hall_wheel_list = wheel;
    portEXIT_CRITICAL(&g_list_mux);

    *handle_out = wheel;
    ESP_LOGI(TAG, "Hall wheel created on GPIO %d", wheel->config.gpio_num);
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

    // 添加到回调链表
    portENTER_CRITICAL(&g_list_mux);
    callback_node_t **head = &wheel->callbacks[event];
    if (*head == NULL) {
        *head = new_node;
    } else {
        callback_node_t *curr = *head;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
    portEXIT_CRITICAL(&g_list_mux);

    return ESP_OK;
}

esp_err_t hall_wheel_delete(hall_wheel_handle_t handle)
{
    HALL_WHEEL_CHECK(handle != NULL, "handle is null", ESP_ERR_INVALID_ARG);
    hall_wheel_dev_t *wheel_to_del = (hall_wheel_dev_t *)handle;

    // 从链表中移除设备
    portENTER_CRITICAL(&g_list_mux);
    if (g_hall_wheel_list == wheel_to_del) {
        g_hall_wheel_list = wheel_to_del->next;
    } else {
        hall_wheel_dev_t *curr = g_hall_wheel_list;
        while (curr && curr->next != wheel_to_del) {
            curr = curr->next;
        }
        if (curr) {
            curr->next = wheel_to_del->next;
        }
    }
    portEXIT_CRITICAL(&g_list_mux);

    // 移除ISR
    gpio_isr_handler_remove(wheel_to_del->config.gpio_num);

    // 删除定时器
    esp_timer_stop(wheel_to_del->timeout_timer);
    esp_timer_delete(wheel_to_del->timeout_timer);

    // 释放所有回调函数
    for (int i = 0; i < HALL_WHEEL_EVENT_MAX; i++) {
        callback_node_t *node = wheel_to_del->callbacks[i];
        while (node) {
            callback_node_t *next = node->next;
            free(node);
            node = next;
        }
    }

    // 释放设备内存
    free(wheel_to_del);

    // 如果没有设备了，销毁全局资源
    portENTER_CRITICAL(&g_list_mux);
    if (g_hall_wheel_list == NULL) {
        hall_wheel_deinit_global();
    }
    portEXIT_CRITICAL(&g_list_mux);

    return ESP_OK;
}

esp_err_t hall_wheel_get_speed(hall_wheel_handle_t handle, uint32_t *speed_pps)
{
    HALL_WHEEL_CHECK(handle != NULL, "handle is null", ESP_ERR_INVALID_ARG);
    HALL_WHEEL_CHECK(speed_pps != NULL, "speed_pps is null", ESP_ERR_INVALID_ARG);

    hall_wheel_dev_t *wheel = (hall_wheel_dev_t *)handle;
    *speed_pps = wheel->state.speed_pps;
    return ESP_OK;
}