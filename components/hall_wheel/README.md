# 霍尔滚轮按键驱动 (Hall Wheel)

这个组件提供了一个用于霍尔滚轮传感器的驱动实现，可以将霍尔滚轮的滚动作为按键输入处理。

## 功能特点

- 支持通过 GPIO 连接的霍尔滚轮设备
- 检测滚轮的滚动行为并转换为按键事件
- 可同时注册多个霍尔滚轮设备
- 使用单一全局定时器扫描管理所有设备，减少资源占用
- 可配置的参数（方波持续时间、检测窗口、最小脉冲数）
- 直接基于 ESP-IDF 原生 GPIO 和定时器组件实现
- 轻量级实现，不依赖第三方按键库

## 工作原理

霍尔滚轮在滚动时会产生一系列脉冲信号（通常是2-3个方波），每个方波的持续时间约为30-40ms。
本驱动通过周期性扫描检测特定时间窗口（默认150ms）内的脉冲数量来判断滚轮是否触发了一次滚动事件。

当检测到超过配置的最小脉冲数（默认2个）时，将触发一个点击事件，调用注册的回调函数。

## 使用方法

### 1. 创建霍尔滚轮设备

```c
#include "hall_wheel.h"

// 定义配置
hall_wheel_config_t wheel_config = {
    .gpio_num = GPIO_NUM_5,      // GPIO引脚
    .active_level = 0,           // 有效电平（0为低电平）
    .wave_duration_ms = 30,      // 方波持续时间（毫秒）
    .detection_window_ms = 150,  // 检测窗口（毫秒）
    .min_pulses = 2              // 最小脉冲数量
};

// 创建设备
hall_wheel_handle_t wheel_handle = NULL;
esp_err_t ret = hall_wheel_create(&wheel_config, &wheel_handle);
if (ret != ESP_OK) {
    // 错误处理
}
```

### 2. 注册回调函数

```c
// 回调函数定义
void wheel_click_callback(hall_wheel_handle_t handle, hall_wheel_event_t event, void *user_data)
{
    // 处理点击事件
    printf("霍尔滚轮已触发！\n");
}

// 注册回调
hall_wheel_register_cb(wheel_handle, HALL_WHEEL_EVENT_CLICK, wheel_click_callback, NULL);
```

### 3. 删除设备

```c
// 程序结束时删除设备
hall_wheel_delete(wheel_handle);
```

## 配置参数说明

- **gpio_num**：连接霍尔滚轮输出的GPIO引脚号
- **active_level**：激活电平（0=低电平，1=高电平）
- **wave_duration_ms**：单个方波的持续时间（毫秒），默认30ms
- **detection_window_ms**：检测窗口时间（毫秒），默认150ms
- **min_pulses**：判定为一次点击的最小脉冲数量，默认2个

## 实现细节

组件使用以下ESP-IDF功能实现霍尔滚轮检测：

1. 单一全局定时器：使用一个ESP定时器周期性扫描所有注册的霍尔滚轮设备
2. 设备链表：所有创建的霍尔滚轮设备通过链表连接并管理
3. 高效GPIO轮询：定期读取GPIO状态并检测电平变化
4. 时间窗口检测：在指定时间窗口内检测脉冲数量来判断滚动事件

这种实现方式相比于每个设备单独使用定时器和任务，大大减少了系统资源占用，特别是在同时使用多个霍尔滚轮设备时。

## 示例

请参考 `examples/hall_wheel_example.c` 获取完整的使用示例。

## 依赖

- ESP-IDF GPIO驱动
- ESP Timer组件
- FreeRTOS 