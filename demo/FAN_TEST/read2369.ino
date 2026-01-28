//2366所有信息读取和串口显示
void READ2366() {

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

  // 输入输出状态以及协议读取（8位）
  byte typeCState = read8BitRegister(REG_DEVICE_STATUS);

  //======================================================================================电量映射以及电量显示

  float BatPow;  // 输出电量（百分比），电量映射，通过电池电压实现

  // 线性映射公式：y = ((x - x_min) / (x_max - x_min)) * y_max
  BatPow = ((batVoltage - VOLTAGE_MIN) / (VOLTAGE_MAX - VOLTAGE_MIN)) * 100.0;

  // 边界处理（电压超出范围时固定为0%或100%）
  BatPow = constrain(BatPow, 0.0, 100.0);

  // 保留1位小数（四舍五入）
  BatPow = round(BatPow * 10) / 10;






  //串口输出数据
  Serial.println("   ");
  Serial.println("   ");
  Serial.println("==============================================================");
  Serial.println("==============================================================");



  //显示充放电状态和协议
  Serial.println("(0x34)输入输出状态以及协议读取:");

  Serial.print("充电 (Bit7): ");
  Serial.println((typeCState & 0x80) ? "充电中" : "待机");

  Serial.print("放电 (Bit6): ");
  Serial.println((typeCState & 0x40) ? "放电中" : "待机");

  Serial.print("PD放电 (Bit5): ");
  Serial.println((typeCState & 0x20) ? "PD放电中" : "待机");

  Serial.print("PD充电 (Bit4): ");
  Serial.println((typeCState & 0x10) ? "PD充电中" : "待机");

  Serial.print("QC充电 (Bit3): ");
  Serial.println((typeCState & 0x08) ? "QC充电中" : "待机");

  Serial.print("QC放电 (Bit2): ");
  Serial.println((typeCState & 0x04) ? "QC放电中" : "待机");

  // 将读取到的时间戳ASCII码值转换为字符并打印
  Serial.print("时间戳数据: ");
  Serial.print(char(timeNode1));
  Serial.print(char(timeNode2));
  Serial.print(char(timeNode3));
  Serial.print(char(timeNode4));
  Serial.println(char(timeNode5));

  // 读取并解析STATE_CTL0寄存器(0x31)
  byte state0 = read8BitRegister(REG_CHARGE_STATUS1);
  Serial.println("(充电状态控制寄存器):");


  Serial.print("(Bit5): ");
  Serial.println((state0 & 0x20) ? "充电中" : "未充电");

  Serial.print("(Bit4): ");
  Serial.println((state0 & 0x10) ? "已充满" : "未充满");

  Serial.print("(Bit3): ");
  Serial.println((state0 & 0x08) ? "放电正常" : "放电停止");

  Serial.print("(Bit2-0): ");
  byte chgState = state0 & 0x07;
  switch (chgState) {
    case 0: Serial.println("待机"); break;
    case 1: Serial.println("涓流充电"); break;
    case 2: Serial.println("恒流充电"); break;
    case 3: Serial.println("恒压充电"); break;
    case 4: Serial.println("充电等待中"); break;
    case 5: Serial.println("充满状态"); break;
    case 6: Serial.println("充电超时"); break;
    default: Serial.println("未知状态");
  }

  // 读取并解析STATE_CTL1寄存器(0x32)
  byte state1 = read8BitRegister(REG_CHARGE_STATUS2);
  Serial.println(" (充电状态控制寄存器):");

  Serial.print("(Bit7-6): ");
  byte inputType = (state1 >> 6) & 0x03;
  switch (inputType) {
    case 0: Serial.println("5V输入充电"); break;
    case 1: Serial.println("高压输入快充"); break;
    default: Serial.println("未知输入");
  }



  Serial.println("   ");
  Serial.print("电量百分比: ");
  Serial.print(BatPow, 1);
  Serial.println(" %");

  Serial.print("电池电压: ");
  Serial.print(batVoltage, 3);
  Serial.println(" V");

  Serial.print("系统电压: ");
  Serial.print(sysVoltage, 3);
  Serial.println(" V");

  Serial.print("电池电流: ");
  Serial.print(batCurrent, 3);
  Serial.println(" A");

  Serial.print("系统电流: ");
  Serial.print(sysCurrent, 3);
  Serial.println(" A");

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
  //====================================================================================

  display.println("READ IP2369  ");
  display.print("Directions:");
  display.print((typeCState & 0x80) ? "IN" : " ");
  display.println((typeCState & 0x40) ? "OUT" : " ");

  //=================================================屏显内容  工作电压


  // display.setCursor(0, 15);      // 设置显示位置(x,y)
  // display.print("Voltage:");     // 显示文本
  display.print(sysVoltage, 0);  // 显示文本
  display.print("V ");

  //=================================================屏显内容  工作电流

  //  display.setCursor(0, 30);      // 设置显示位置(x,y)
  //display.print("Current:");     // 显示文本
  display.print(sysCurrent, 1);  // 显示文本
  display.print("A ");

  //=================================================屏显内容  实时功率

  // display.setCursor(0, 45);    // 设置显示位置(x,y)
  // display.print("Power:");     // 显示文本
  display.print(sysPower, 1);  // 显示文本
  display.print("W ");

  //=================================================屏显内容  充放电显示


  //=================================================屏显内容  充放电显示

  // display.print("TEMP:");         // 显示文本
  display.print(temperature, 1);  // 显示文本
  display.println("C");



  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();

  V = busvoltage + (shuntvoltage / 1000);
  C = (current_mA / 100) - 0.3;      //单位换算，mA换算成A，并保留两位小数(原适用于0.1Ω电阻，所以需要除以1000，但需要测更大功率，所以更换了0.01Ω电阻，所以需要放大十倍，所以此处只除以100)
  P = (power_mW / 100) - (0.3 * V);  //单位换算，mW换算成W，并保留两位小数


  display.println(V, 2);
  display.println(abs(C), 1);  //显示负载功率
  display.println(abs(P), 2);

  if (digitalRead(UP_PIN) == LOW) { display.print("UP "); }
  //  if (digitalRead(UP_PIN) == HIGH) {display.print("UP1");}

  if (digitalRead(DOWN_PIN) == LOW) { display.print("DOWN "); }
  // if (digitalRead(DOWN_PIN) == HIGH) {display.print("DOWN1:");}

  if (digitalRead(LEFT_PIN) == LOW) { display.print("LEFT "); }
  //  if (digitalRead(LEFT_PIN) == HIGH) {display.print("LEFT1");}

  if (digitalRead(RIGHT_PIN) == LOW) { display.println("RIGHT "); }
  // if (digitalRead(RIGHT_PIN) == HIGH) {display.print("RIGHT1");}
}