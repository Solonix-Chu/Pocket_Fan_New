void PAGE0() {

  //若2366 int引脚拉低，且超时10s，则进入休眠【此页死循环，无法从loop进入休眠】
  //休眠检测
  TIMEEND = millis() - TIMESTART;                         //实时更新时间，此时间每次按下按键后重置
  if (digitalRead(WAKE_PIN) == LOW && TIMEEND > 10000) {  //若2366提供低电平，且唤醒超过10s，才进入休眠
    delay(20);                                            // 20ms去抖延迟
    sleep();
  } else if (digitalRead(WAKE_PIN) == LOW && TIMEEND < 10000) {  //若int为低电平，但是又按下了功能键，则临时设置Weak pin为输出引脚，并设置其为高电平，以实现保持2366被唤醒不休眠。
    pinMode(WAKE_PIN, OUTPUT);                                   //设置为输出引脚
    digitalWrite(WAKE_PIN, HIGH);                                //让2366保持唤醒状态
    delay(50);                                                   // 确保信号被外部设备接收
    pinMode(WAKE_PIN, INPUT);                                    //设置为输入引脚，以便让2366唤醒ESP32
    delay(50);                                                   // 等待引脚模式切换完成
  }


  Serial.print("表情序号：");
  Serial.println(emotion);

  //若按键按下，则页面代码+1然后回到loop循环，否则本函数死循环
  if (digitalRead(ENTER_PIN) == LOW) {
    delay(200);
    PAGE = (PAGE + 1) % PAGE_SIZE;  //页面代码更新，从loop跳到change再到新的页面
    TIMESTART = millis();           //重新计时
    loop();
  } else {
    // 更新面部显示
    face->Update();
    PAGE0();  //在此页面循环
  }
}

void Display_Face() {
  if (emotion == 0) {
    face->Expression.GoTo_Normal();
  } else if (emotion == 1) {
    face->Expression.GoTo_Angry();
  } else if (emotion == 2) {
    Serial.println("ready GLEE");
    face->Expression.GoTo_Glee();
  } else if (emotion == 3) {
    face->Expression.GoTo_Happy();
  } else if (emotion == 4) {
    Serial.println("ready sad");
    face->Expression.GoTo_Sad();
  } else if (emotion == 5) {
    face->Expression.GoTo_Worried();
  } else if (emotion == 6) {
    face->Expression.GoTo_Focused();
  } else if (emotion == 7) {
    face->Expression.GoTo_Annoyed();
  } else if (emotion == 8) {
    face->Expression.GoTo_Surprised();
  } else if (emotion == 9) {
    face->Expression.GoTo_Skeptic();
  } else if (emotion == 10) {
    face->Expression.GoTo_Frustrated();
  } else if (emotion == 11) {
    face->Expression.GoTo_Unimpressed();
  } else if (emotion == 12) {
    face->Expression.GoTo_Sleepy();
  } else if (emotion == 13) {
    face->Expression.GoTo_Suspicious();
  } else if (emotion == 14) {
    face->Expression.GoTo_Squint();
  } else if (emotion == 15) {
    face->Expression.GoTo_Furious();
  } else if (emotion == 16) {
    face->Expression.GoTo_Scared();
  } else if (emotion == 17) {
    face->Expression.GoTo_Awe();
  }
}