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

  // 根据功率状态控制计时
  if (sysPower >= 1.0) {



    Serial.print("功率大于1，实际值为：");
    Serial.println(sysPower);

    unsigned long currentMillis = millis();
    // 检查是否达到更新时间间隔
    if (currentMillis - LEDMillis >= interval) {
      // 记录上次更新时间
      LEDMillis = currentMillis;

      // 设置LED亮度
      analogWrite(8, brightness);
      Serial.print("LED亮度值为：");
      Serial.println(brightness);
      // 改变亮度值
      brightness = brightness + fadeAmount;

      // 亮度达到上限或下限时反转变化方向
      if (brightness <= 0 || brightness >= 255) {
        fadeAmount = -fadeAmount;
      }
    }





    startTiming();  //开始计时，记录开始时间点。设置计时标志，记录当前millis()作为起始时间。
  } else {
    stopTiming();  //停止计时，更新累计时间并保存。计算本次计时的时长，累加到总时间，调用saveAccumulatedTime()。
  }
  // 更新计时
  updateTiming();
  //串口打印累计时间
  printCurrentTime();      //实时打印时间
  printAccumulatedTime();  //打印已经写入flash的时间

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



  //=================================================屏显内容  电量映射以及电量显示

  // 计算需要显示的电量格数量（每个区间对应1格，25%为一个单位）
  int gridCount = 0;
  if (BatPow > 75) gridCount = 4;
  else if (BatPow > 50) gridCount = 3;
  else if (BatPow > 25) gridCount = 2;
  else gridCount = 1;  // 包含BatPow<=25的情况

  // 从右到左绘制电量格（每个格子宽度2px，间距1px，总间隔3px）
  for (int i = 0; i < gridCount; i++) {
    int xPos = 124 - 3 * i;             // 计算每个格子的x坐标（124, 121, 118, 115）
    u8g2.drawXBM(xPos, 5, 2, 3, DIAN);  // 绘制位图，电池电量小方块
  }
  //=================================================屏显内容  工作电压

  u8g2.setFont(u8g2_font_helvB12_tr);  // 设置字体，选择一个适合显示英文的字体
  u8g2.setCursor(0, 16);               // 设置显示位置(x,y)
  u8g2.print(sysVoltage, 0);           // 显示文本
  u8g2.println("V");

  //=================================================屏显内容  工作电流

  u8g2.setCursor(0, 30);      // 设置显示位置(x,y)
  u8g2.print(sysCurrent, 2);  // 显示文本
  u8g2.println("A");

  //=================================================屏显内容  实时功率

  u8g2.setCursor(0, 44);    // 设置显示位置(x,y)
  u8g2.print(sysPower, 1);  // 显示文本
  u8g2.println("W");

  //=================================================屏显内容  电量百分比
  u8g2.setFont(u8g2_font_helvB18_tr);  // 设置字体，电量百分比用更大的字体
  if (BatPow > 99) {
    u8g2.setCursor(68, 21);  // 设置显示位置(x,y)
    u8g2.print("100");       // 显示文本
  } else {
    u8g2.setCursor(80, 21);  // 设置显示位置(x,y)
    u8g2.print(BatPow, 0);   // 显示文本
  }

  //=================================================屏显内容  功率映射绘制直线

  // 限制功率值范围【更改此值可以改定功率最大范围】
  if (sysPower < 0) sysPower = 0;
  else if (sysPower > 100) sysPower = 100;

  // 计算进度条长度（适配屏幕宽度）
  int lineLength = static_cast<int>((sysPower / 100.0) * u8g2.getDisplayWidth());

  // 绘制进度条
  u8g2.drawLine(0, 63, lineLength, 63);  //起点0，63，终点lineLength, 63

  //=================================================屏显内容  充放电方向显示

  if (state0 & 0x20) {                   // 充电判断，判断第5位是否为1，是则显示向下箭头
    u8g2.drawXBMP(57, 24, 9, 15, PIN);   //坐标，分辨率，数组名字，写入主框图
  } else if (state0 & 0x08) {            // 放电判断，判断第3位是否为1，是则显示向上箭头
    u8g2.drawXBMP(57, 24, 9, 15, POUT);  //坐标，分辨率，数组名字，写入主框图
  }

  //=================================================屏显内容  快充指示显示

  if (sysVoltage > 6) {                  // 快充判断，当握手电压超过6V时，则认为识别到了快充，如9V,12V,等
    u8g2.drawXBMP(100, 0, 9, 13, PDQC);  //坐标，分辨率，数组名字，写入主框图
  }
}