//2366详细数据第二页，电量，功率，温度，SN
void PAGE3() {
  u8g2.clearBuffer();
  u8g2.drawXBMP(0, 0, 127, 64, MORE2);  //坐标，分辨率，数组名字

  //======================================================================================电量映射以及电量显示
  // 读取电池电压（单位：mV）
  int16_t batVolt = read16BitRegister(REG_BAT_VOLT);
  float batVoltage = batVolt / 1000.0;

  float BatPow;  // 输出电量（百分比），电量映射，通过电池电压实现

  // 线性映射公式：y = ((x - x_min) / (x_max - x_min)) * y_max
  BatPow = ((batVoltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN)) * 100.0;

  // 边界处理（电压超出范围时固定为0%或100%）
  BatPow = constrain(BatPow, 0.0, 100.0);

  // 保留1位小数（四舍五入）
  BatPow = round(BatPow * 10) / 10;

  // 读取功率（单位：10mW → W）
  int16_t power = read16BitRegister(REG_POWER);
  float sysPower = power * 0.01;


  //读取温度数据
  // 读取NTC输出电流设置
  byte ntcIADCData = read8BitRegister(REG_TEMP_CURR);
  float ntcCurrent = (ntcIADCData & 0x80) ? 80.0e-6 : 20.0e-6;  // A，根据bit7选择80uA或20uA

  // 读取NTC电压
  uint16_t vNTC = read16BitRegister(REG_TEMP_VOLT_LOW);
  float NTCvoltage = vNTC * 3.3 / 4095.0;  // 假设ADC为12位（根据手册实际分辨率调整）

  // 将NTC电压转换为温度（使用Steinhart-Hart方程）
  float temperature = convertVoltageToTemperature(NTCvoltage, ntcCurrent);

  // 读取时间戳寄存器数据
  byte timeNode1 = read8BitRegister(REG_TIMENODE1);
  byte timeNode2 = read8BitRegister(REG_TIMENODE2);
  byte timeNode3 = read8BitRegister(REG_TIMENODE3);
  byte timeNode4 = read8BitRegister(REG_TIMENODE4);
  byte timeNode5 = read8BitRegister(REG_TIMENODE5);


  //串口内容显示
  Serial.print("电量百分比: ");
  Serial.print(BatPow, 3);
  Serial.println(" %");

  Serial.print("系统功率: ");
  Serial.print(sysPower, 2);
  Serial.println(" W");

  // 温度输出结果
  Serial.print("NTC电流: ");
  Serial.print(ntcCurrent * 1.0e6, 1);  // 转换为uA
  Serial.print("uA, 电压: ");
  Serial.print(NTCvoltage, 3);
  Serial.print("V, 温度: ");
  Serial.print(temperature, 1);
  Serial.println("°C");

  // 将读取到的时间戳ASCII码值转换为字符并打印
  Serial.print("时间戳数据: ");
  Serial.print(char(timeNode1));
  Serial.print(char(timeNode2));
  Serial.print(char(timeNode3));
  Serial.print(char(timeNode4));
  Serial.println(char(timeNode5));


  //OLED内容显示
  u8g2.setFont(u8g2_font_helvB12_tr);  // 设置字体，选择一个适合显示英文的字体

  //===================================== 屏显内容  实时电量

  u8g2.setCursor(74, 1);  // 设置显示位置(x,y)
  u8g2.print(BatPow, 2);  // 显示文本
  u8g2.println("%");

  //===================================== 屏显内容  系统功率显示

  u8g2.setCursor(74, 18);   // 设置显示位置(x,y)
  u8g2.print(sysPower, 2);  // 显示文本
  u8g2.println("W");

  //===================================== 屏显内容  电池温度显示

  u8g2.setCursor(74, 35);      // 设置显示位置(x,y)
  u8g2.print(temperature, 2);  // 显示文本
  //===================================== 屏显内容  SN码显示【2366的时间戳】

  u8g2.setCursor(74, 51);  // 设置显示位置(x,y)
  u8g2.print(char(timeNode1));
  u8g2.print(char(timeNode2));
  u8g2.print(char(timeNode3));
  u8g2.print(char(timeNode4));
  u8g2.println(char(timeNode5));
  u8g2.sendBuffer();
}