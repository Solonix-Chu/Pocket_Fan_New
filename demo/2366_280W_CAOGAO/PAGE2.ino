//显示2366详细数据第一页,电池电压电流，系统电压电流

void PAGE2() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 69, 64, MORE1);  //坐标，分辨率，数组名字

  // 读取电池电压（单位：mV）
  int16_t batVolt = read16BitRegister(REG_BAT_VOLT);
  float batVoltage = batVolt / 1000.0;

  // 读取系统电压（单位：mV）
  int16_t sysVolt = read16BitRegister(REG_SYS_VOLT);
  float sysVoltage = sysVolt / 1000.0;

  // 读取电池电流（单位：mA，注意补码处理）
  int16_t batCurr = read16BitRegister(REG_BAT_CURR);
  float batCurrent = static_cast<int16_t>(batCurr) / 1000.0;  // 处理有符号数

  // 读取系统电流（单位：mA）
  int16_t sysCurr = read16BitRegister(REG_SYS_CURR);
  float sysCurrent = static_cast<int16_t>(sysCurr) / 1000.0;

  //串口内容显示
  Serial.print("电池电压: ");
  Serial.print(batVoltage, 3);
  Serial.println(" V");

  Serial.print("电池电流: ");
  Serial.print(batCurrent, 3);
  Serial.println(" A");

  Serial.print("系统电压: ");
  Serial.print(sysVoltage, 3);
  Serial.println(" V");

  Serial.print("系统电流: ");
  Serial.print(sysCurrent, 3);
  Serial.println(" A");

  //OLED内容显示
  u8g2.setFont(u8g2_font_helvB12_tr);  // 设置字体，选择一个适合显示英文的字体

  //===================================== 屏显内容  电池电压显示
  
  u8g2.setCursor(75, 1);               // 设置显示位置(x,y)
  u8g2.print(batVoltage, 2);           // 显示文本
  u8g2.println("V");

  //===================================== 屏显内容  电池电流显示
  
  u8g2.setCursor(75, 18);               // 设置显示位置(x,y)
  u8g2.print(batCurrent, 2);           // 显示文本
  u8g2.println("A");

    //===================================== 屏显内容  系统电压显示
  
  u8g2.setCursor(75, 35);               // 设置显示位置(x,y)
  u8g2.print(sysVoltage, 2);           // 显示文本
  u8g2.println("V");

    //===================================== 屏显内容  系统电流显示
  
  u8g2.setCursor(75, 51);               // 设置显示位置(x,y)
  u8g2.print(sysCurrent, 2);           // 显示文本
  u8g2.println("A");
  
  u8g2.sendBuffer();
}