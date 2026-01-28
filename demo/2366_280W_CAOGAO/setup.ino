void setup() {

  //串口初始化
  Serial.begin(115200);

   //I2C总线初始化
  Wire.begin(SDA_PIN, SCL_PIN);

  // 设置I2C时钟为200kHz
  Wire.setClock(200000);


  // 配置引脚：输入模式 + 内置下拉电阻（默认低电平）
  pinMode(WAKE_PIN, INPUT);
  pinMode(8, OUTPUT);                //LED引脚定义
  pinMode(ENTER_PIN, INPUT_PULLUP);  //按键默认上拉


  // 获取唤醒原因
  esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();

  // 区分不同唤醒源
  switch (cause) {
    case ESP_SLEEP_WAKEUP_GPIO:
      {
        // 读取引脚状态判断具体唤醒源
        bool wakePinState = gpio_get_level((gpio_num_t)WAKE_PIN);
        bool enterPinState = gpio_get_level((gpio_num_t)ENTER_PIN);

        if (wakePinState == 1) {  // WAKE_PIN触发（高电平）
          Serial.println("唤醒源: 2366主动唤醒");
        } else if (enterPinState == 0) {  // ENTER_PIN触发（低电平）
          Serial.println("唤醒源: 按键触发主动唤醒");
        }
      }
    default:
      Serial.println("唤醒源: 其他原因 (首次启动或复位)");
      break;
  }







  //单次计时归零，休眠时间不算
 TIMESTART = millis(); 
TIMEEND = millis() - TIMESTART; 

  // 初始化EEPROM【时间统计计时器】
  if (!initEEPROM()) {
    Serial.println("错误: EEPROM初始化失败，使用默认值");
    accumulatedMillis = 0;
  } else {
    // 从EEPROM加载已累计的时间
    loadAccumulatedTime();  //从 EEPROM 中读取之前保存的累计时间数据。验证数据完整性后加载到内存。
  }
  printAccumulatedTime();  //格式化输出累计时间（小时 + 分钟）。将毫秒转换为小时和分钟，通过串口打印。

  // 初始化OLED显示屏
  u8g2.begin();                        // 初始化U8g2对象
  u8g2.setFont(u8g2_font_helvB12_tr);  // 设置字体
  u8g2.setFontPosTop();                // 设置文字定位方式为顶部对齐



  //=======================================================表情包基本设置
  emotion = random(min_value, max_value);  //每次复位更新一次随机数

  // 创建一个新的面部对象，设置屏幕宽度、高度和眼睛大小
  face = new Face(/* screenWidth = */ 128, /* screenHeight = */ 64, /* eyeSize = */ 50);//默认为128  64  40

  // 设置当前表情为正常
  //face->Expression.GoTo_Normal();


  // 自动在不同的行为之间切换（基于分配给每种情绪的权重随机选择新的行为）
  face->RandomBehavior = true;  //true为开启此功能，false为关闭

  // 自动眨眼
  face->RandomBlink = true;  //true为开启此功能，false为关闭

  // 设置眨眼的间隔时间
  face->Blink.Timer.SetIntervalMillis(3000);

  // 自动选择新的随机方向看
  face->RandomLook = true;  //true为开启此功能，false为关闭

  // 开机图片
  u8g2.clearBuffer();
  u8g2.drawXBMP(26, 10, 76, 45, DJI);  //开机图片，DJI
  u8g2.sendBuffer();
  PAGE = 1;  //默认显示表情包界面
  delay(500);//开机图片显示持续时间
}