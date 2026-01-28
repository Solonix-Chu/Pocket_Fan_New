

void setup() {
  //串口初始化
  Serial.begin(115200);

  //I2C总线初始化
  Wire.begin(SDA_PIN, SCL_PIN);
  
  // 设置I2C时钟为200kHz
  Wire.setClock(200000);

  pinMode(35, OUTPUT);
  pinMode(36, OUTPUT);
  pinMode(37, OUTPUT);
  pinMode(38, OUTPUT);

  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(WAKE_PIN, OUTPUT);

  pinMode(ESC_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);         // 配置微动开关引脚：内部上拉输入（开关断开时为HIGH，闭合时为LOW）
  pinMode(KEY_PIN, INPUT_PULLUP);  //功能按键定义
  digitalWrite(EN_PIN, HIGH);
  digitalWrite(ESC_PIN, HIGH);
  digitalWrite(WAKE_PIN, HIGH);
  digitalWrite(35, HIGH);
  digitalWrite(36, HIGH);
  digitalWrite(37, HIGH);
  digitalWrite(38, HIGH);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // 初始化OLED屏幕
  display.setRotation(2);                     //屏幕显示旋转180°，1=90°，2=180°
  display.clearDisplay();                     // 清屏

  // 显示初始化信息
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("JEF.LEE Test");
  display.println("GPIO8=SDA, GPIO9=SCL");
  display.println("Initializing...");
  display.display();
  delay(1000);
  digitalWrite(35, LOW);
  digitalWrite(36, LOW);
  digitalWrite(37, LOW);
  digitalWrite(38, LOW);



  if (digitalRead(KEY_PIN) == LOW) {  //若2366提供低电平，且唤醒超过10s，才进入休眠
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("START......");
    display.display();
    delay(1000);
    if (digitalRead(KEY_PIN) == LOW) {
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("START SUCCERS");
      display.display();
      delay(1000);
      digitalWrite(EN_PIN, HIGH);
    } else {

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("SHUTDOWN......");
      display.display();
      delay(1000);
      digitalWrite(EN_PIN, LOW);
      digitalWrite(ESC_PIN, LOW);
    }
  } else {

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("SHUTDOWN......");
    display.display();
    delay(1000);
    digitalWrite(EN_PIN, LOW);
    digitalWrite(ESC_PIN, LOW);
  }



  // 启动提示（明确功能）
  Serial.println("=== 暴力风扇启动 ===");
  Serial.println("启动成功");
  Serial.println("=======================\n");





    // 初始化INA219
  Serial.println("Initializing INA219...");
  if (!ina219.begin(&Wire)) {  // 指定使用Wire总线
    Serial.println("Failed to find INA219! Check wiring or I2C address");
    while (1) delay(100); // 初始化失败则停止
  }
  
  // 根据模块规格进行校准
#ifdef CALIBRATION_32V_2A
  ina219.setCalibration_32V_2A();  // 32V量程，最大2A电流
  Serial.println("INA219 calibrated for 32V/2A");
#else
  ina219.setCalibration_16V_400mA(); // 16V量程，最大400mA电流
  Serial.println("INA219 calibrated for 16V/400mA");
#endif
  
  Serial.println("INA219 initialized successfully!");
  Serial.println("--------------------------------------");
}
