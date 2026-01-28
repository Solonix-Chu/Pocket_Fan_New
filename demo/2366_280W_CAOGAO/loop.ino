void loop() {

  //======================================================================================自动休眠
  Serial.print("2366INT状态： ");
  Serial.println(digitalRead(WAKE_PIN));


  // 读取按键键值并且赋值PAGE
  if (digitalRead(ENTER_PIN) == LOW) {
    delay(200);                     //消抖
    PAGE = (PAGE + 1) % PAGE_SIZE;  //页面号+1
                                    //PAGE = (PAGE - 1 + PAGE_SIZE) % PAGE_SIZE;//页面号-1
    TIMESTART = millis();           //重新计时
  }

  TIMEEND = millis() - TIMESTART;  //实时更新时间，此时间每次按下按键后重置

  //休眠检测
  if (digitalRead(WAKE_PIN) == LOW && TIMEEND > 10000) {  //若2366提供低电平，且唤醒超过10s，才进入休眠
   // delay(20);                                            // 20ms去抖延迟
   // sleep();
  } else if (digitalRead(WAKE_PIN) == LOW && TIMEEND < 10000) {
    pinMode(WAKE_PIN, OUTPUT);
    digitalWrite(WAKE_PIN, HIGH);
    delay(50);  // 确保信号被外部设备接收
    pinMode(WAKE_PIN, INPUT);
  }



  Serial.print("单次时间统计：");
  Serial.println(TIMEEND);







  // 处理串口输入信息，当串口有信息输入时，则跳到processCommand函数进行分辨、处理。
  while (Serial.available() > 0) {
    char c = Serial.read();
    if (c == '\n' || commandIndex >= MAX_COMMAND_LENGTH - 1) {
      commandBuffer[commandIndex] = '\0';  // 字符串结束符
      processCommand();                    //转到processCommand()函数，处理接收到的命令
      commandIndex = 0;                    // 重置缓冲区
    } else {
      commandBuffer[commandIndex++] = c;
    }
  }



  Serial.print("millis：");
  Serial.println(millis());

  Serial.print("页面号：");
  Serial.println(PAGE);

  Serial.print("表情序号：");
  Serial.println(emotion);
  PAGE_CHANGE();
  //delay(200);  // 延长读取间隔，避免总线拥塞
}  //loop函数小尾巴括号
