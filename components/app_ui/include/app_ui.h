// components/ui/include/ui.h

#ifndef APP_UI_H
#define APP_UI_H

/**
 * @brief 初始化UI组件
 * 在显示任何内容之前调用此函数
 */
void ui_init(void);

/**
 * @brief UI主处理函数
 * 应该在主循环或一个专用的RTOS任务中被周期性调用
 * 它负责处理状态、动画和重绘屏幕
 */
void ui_proc(void);

void ui_task(void *pvParameters);

#endif // APP_UI_H