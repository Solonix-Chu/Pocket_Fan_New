void sleep() {
  u8g2.clearBuffer();  //清除缓存
  u8g2.sendBuffer();   //更新缓存

  // 检测到低电平时（默认状态）进入深度休眠（外部拉高时唤醒）
  Serial.println("【触发休眠】检测到低电平，配置高电平唤醒条件");

  // 使能GPIO唤醒功能，启用睡眠功能
  esp_sleep_enable_gpio_wakeup();

  // 配置唤醒引脚和触发条件（高电平触发）
  gpio_wakeup_enable((gpio_num_t)WAKE_PIN, GPIO_INTR_HIGH_LEVEL);  //当2366int为高电平时，唤醒ESP32
  gpio_wakeup_enable((gpio_num_t)ENTER_PIN, GPIO_INTR_LOW_LEVEL);  //当按键为低电平时，唤醒2366


  // 关闭I2C总线（假设使用I2C0）
  Wire.end();
  // 关闭串口以降低休眠功耗（可选）
  Serial.end();
  //digitalWrite(8, HIGH);  //LED灭
  // 进入深度休眠，等待引脚变为高电平唤醒
  //esp_deep_sleep_start();  // 进入深度睡眠【唤醒后会先执行setup函数，然后进入loop】深度睡眠暂时无法唤醒，需要优化
  esp_light_sleep_start();  //进入轻睡眠【唤醒后从此处继续执行代码】
  ESP.restart();            // 立即重启系统，即软件复位
                            // setup();                  //搭配轻睡眠使用
}
