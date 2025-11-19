# 霍尔滚轮驱动 (Hall Wheel)

该组件提供了一个基于中断的霍尔滚轮传感器驱动，经过重构以支持高速滚动检测、速度获取和更可靠的事件处理。

## 功能特点

- **中断驱动**：使用GPIO中断精确捕捉每一次电平变化，能够处理高速滚动（如脉冲宽度低至2.5ms）的场景。
- **滚动会话管理**：通过状态机管理滚动过程，有效过滤由微小误触产生的噪声信号。
- **速度检测**：内置滚动速度计算功能，可随时通过 `hall_wheel_get_speed()` 获取当前速度（单位：脉冲/秒）。
- **丰富的事件**：除了常规的 `CLICK` 事件，还提供 `SCROLL_START` 和 `SCROLL_END` 事件，方便应用层处理完整的滚动生命周期。
- **资源高效**：所有设备实例共享一个处理任务和中断服务，最大限度减少系统资源占用。
- **可配置性**：提供防抖、滚动超时和触发阈值等参数，可灵活适配不同硬件。
- **保持API兼容**：核心的 `hall_wheel_register_cb` 接口保持不变，便于旧项目迁移。

## 工作原理

驱动为每个霍尔滚轮设备注册一个 `GPIO_INTR_ANYEDGE` 中断。当滚轮滚动产生脉冲时，ISR（中断服务程序）会立即捕捉到信号边沿，并将带有时间戳的事件推入一个全局队列。

一个独立的后台任务从队列中获取事件，并根据脉冲的时间间隔进行处理：
1.  **防抖**：通过 `wave_duration_ms` 配置过滤掉间隔过短的噪声信号。
2.  **会话启动**：当在短时间内检测到的脉冲数达到 `min_pulses` 阈值时，启动一个滚动会话，并触发 `SCROLL_START` 事件。
3.  **点击事件**：在滚动会话期间，每次有效的脉冲都会触发一次 `CLICK` 事件。
4.  **速度计算**：根据最近几次脉冲之间的时间间隔，计算出平均滚动速度。
5.  **会话结束**：如果在 `detection_window_ms` 时间内没有新的脉冲，则认为滚动结束，触发 `SCROLL_END` 事件，并将状态重置。

## 使用方法

### 1. 创建霍尔滚轮设备

```c
#include "hall_wheel.h"

// 定义配置
hall_wheel_config_t wheel_config = {
    .gpio_num = GPIO_NUM_5,      // GPIO引脚
    .active_level = 0,           // 有效电平（0为低电平）
    .wave_duration_ms = 2,       // 防抖时间(ms)，对于高速滚轮，设为1-2ms
    .detection_window_ms = 100,  // 滚动会话超时(ms)
    .min_pulses = 3              // 最小脉冲数，用于启动滚动
};

// 创建设备
hall_wheel_handle_t wheel_handle = NULL;
esp_err_t ret = hall_wheel_create(&wheel_config, &wheel_handle);
if (ret != ESP_OK) {
    // 错误处理
}
```

### 2. 注册回调函数

可以为不同的事件注册回调。

```c
// 回调函数定义
void wheel_event_callback(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data)
{
    switch (event) {
        case HALL_WHEEL_EVENT_CLICK:
            ESP_LOGI("Wheel", "滚轮 '%s' 产生一次点击", (const char *)user_data);
            
            // 获取当前速度
            uint32_t speed = 0;
            hall_wheel_get_speed(handle, &speed);
            ESP_LOGI("Wheel", "当前速度: %d PPS", speed);
            break;
        case HALL_WHEEL_EVENT_SCROLL_START:
            ESP_LOGI("Wheel", "滚轮 '%s' 开始滚动", (const char *)user_data);
            break;
        case HALL_WHEEL_EVENT_SCROLL_END:
            ESP_LOGI("Wheel", "滚轮 '%s' 停止滚动", (const char *)user_data);
            break;
        default:
            break;
    }
}

// 注册回调
hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_CLICK, wheel_event_callback, "滚轮1");
hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_SCROLL_START, wheel_event_callback, "滚轮1");
hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_SCROLL_END, wheel_event_callback, "滚轮1");
```

### 3. 获取滚动速度

可以在任何时候主动获取速度。

```c
uint32_t current_speed = 0;
hall_wheel_get_speed(wheel_handle, &current_speed);
// 使用 current_speed
```

### 4. 删除设备

```c
// 程序结束时删除设备
hall_wheel_delete(wheel_handle);
```

## 配置参数说明

- **gpio_num**：连接霍尔滚轮输出的GPIO引脚号。
- **active_level**：激活电平（0=低电平，1=高电平）。
- **wave_duration_ms**：**（重要）** 用于防抖的最小脉冲间隔（单位：毫秒）。如果两次电平变化间隔小于此值，则被视为噪声。对于高速滚轮，建议设为 `1` 或 `2`。
- **detection_window_ms**：滚动会话的超时时间（单位：毫秒）。若在此时间内无新脉冲，则认为滚动结束。
- **min_pulses**：启动一次滚动会话所需的最小脉冲数。此值有助于过滤掉因意外触碰而产生的单个脉冲。

## 示例

请参考 `examples/hall_wheel_example.c` 获取完整的使用示例。

## 依赖

- ESP-IDF GPIO 驱动
- ESP Timer 组件
- FreeRTOS