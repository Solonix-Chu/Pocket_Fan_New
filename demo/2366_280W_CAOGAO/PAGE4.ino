//2366详细数据第三页，累计使用时间，设备循环次数。
void PAGE4() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 1, 101, 46, MORE3);  //坐标，分辨率，数组名字

  // 读取功率（单位：10mW → W）
  int16_t power = read16BitRegister(REG_POWER);
  float sysPower = power * 0.01;

/*
  // 根据功率状态控制计时
  if (sysPower >= 1.0) {
    TIMESTART = millis();  //当功率大于1时，开始计时。
     TIMEEND = millis() - TIMESTART;//当前时间减去开始时间等于记录的时间
  }else{
    TIMEEND = TIMEEND;//当功率小于1时，则计时暂停
  }
*/




  // 计算总秒数（提高精度）【历史累计时间】
  uint32_t totalSeconds = accumulatedMillis / 1000;

  // 分解为小时、分钟、秒【历史累计时间】
  uint32_t hours = totalSeconds / 3600;
  uint32_t minutes = (totalSeconds % 3600) / 60;
  uint32_t seconds = totalSeconds % 60;

  // 格式化输出（带前导零）【历史累计时间】
  Serial.print("==================分隔符================== ");
  Serial.print("历史累计时间: ");
  Serial.print(hours);
  Serial.print("小时 ");

  if (minutes < 10) Serial.print("0");  // 前导零
  Serial.print(minutes);
  Serial.print("分 ");

  if (seconds < 10) Serial.print("0");  // 前导零
  Serial.print(seconds);
  Serial.println("秒");


  //===================================== 屏显内容  【历史累计时间】
  //OLED内容显示
  u8g2.setFont(u8g2_font_helvB12_tr);  // 设置字体，选择一个适合显示英文的字体

  u8g2.setCursor(0, 17);  // 设置显示位置(x,y)
  u8g2.print(hours);
  u8g2.print("H ");

  u8g2.print(minutes);
  u8g2.println("Min ");

  //===================================== 屏显内容  【单次累计时间】

  u8g2.setCursor(0, 51);  // 设置显示位置(x,y)
  u8g2.print(millis()/1000);
  u8g2.print("s ");

  u8g2.sendBuffer();
}