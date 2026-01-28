// 清除累计时间并重置EEPROM
void clear() {
  // 重置计时变量
  accumulatedMillis = 0;
  previousMillis = millis();
  
  // 清除EEPROM中的数据
  TimingData data = {0};
  data.crc = calculateCRC(&data.accumulatedMinutes, sizeof(data.accumulatedMinutes));
  
  // 写入数据到EEPROM
  for (size_t i = 0; i < sizeof(TimingData); i++) {
    EEPROM.write(EEPROM_START_ADDRESS + i, ((uint8_t*)&data)[i]);
  }
  
  // 提交更改
  EEPROM.commit();
  
  // 如果正在计时，重置状态
  if (isTiming) {
    previousMillis = millis();
  }
}

// 初始化EEPROM
bool initEEPROM() {
  // 初始化EEPROM库，分配指定大小的空间
  bool success = EEPROM.begin(EEPROM_SIZE);
  
  if (success) {
    Serial.println("EEPROM初始化成功");
    return true;
  } else {
    Serial.println("错误: EEPROM初始化失败");
    return false;
  }
}

// 从EEPROM加载累计时间
void loadAccumulatedTime() {
  TimingData data;
  
  // 从EEPROM读取数据
  for (size_t i = 0; i < sizeof(TimingData); i++) {
    ((uint8_t*)&data)[i] = EEPROM.read(EEPROM_START_ADDRESS + i);
  }
  
  // 计算CRC并验证
  uint32_t calculatedCRC = calculateCRC(&data.accumulatedMinutes, sizeof(data.accumulatedMinutes));
  
  if (calculatedCRC == data.crc) {
    accumulatedMillis = data.accumulatedMinutes * 60000;
    Serial.print("已加载累计时间: ");
    printAccumulatedTime();
  } else {
    Serial.println("错误: EEPROM数据CRC校验失败，使用默认值");
    accumulatedMillis = 0;
  }
}

// 保存累计时间到EEPROM
void saveAccumulatedTime() {
  static uint32_t lastSavedMinutes = 0;
  uint32_t currentMinutes = accumulatedMillis / 60000;
  
  // 只有当分钟数确实变化时才写入，避免不必要的写入
  if (currentMinutes == lastSavedMinutes) {
    return;
  }
  
  TimingData data;
  data.accumulatedMinutes = currentMinutes;
  data.crc = calculateCRC(&data.accumulatedMinutes, sizeof(data.accumulatedMinutes));
  
  // 写入数据到EEPROM
  for (size_t i = 0; i < sizeof(TimingData); i++) {
    EEPROM.write(EEPROM_START_ADDRESS + i, ((uint8_t*)&data)[i]);
  }
  
  // 提交更改到Flash
  bool success = EEPROM.commit();
  
  if (success) {
    lastSavedMinutes = currentMinutes;
    Serial.print("已成功保存累计时间: ");
    printAccumulatedTime();
  } else {
    Serial.println("错误: 保存累计时间失败");
  }
}

// 打印当前计时状态（包含实时更新）
void printCurrentTime() {
  if (isTiming) {
    // 计算当前实时时间（包含正在计时的部分）
    unsigned long currentTotalMillis = accumulatedMillis + (millis() - previousMillis);
    
    // 先转换为总分钟数
    uint32_t totalMinutes = currentTotalMillis / 60000;
    
    // 计算小时、分钟和秒
    uint32_t hours = totalMinutes / 60;
    uint32_t minutes = totalMinutes % 60;
    uint32_t seconds = (currentTotalMillis / 1000) % 60;

    Serial.print("正在计时 - 累计: ");
    Serial.print(hours);
    Serial.print("小时 ");
    Serial.print(minutes);
    Serial.print("分 ");
    Serial.print(seconds);
    Serial.println("秒");
  } 
}

// 打印累计时间（格式化输出，精确到秒）
void printAccumulatedTime() {
  // 计算总秒数（提高精度）
  uint32_t totalSeconds = accumulatedMillis / 1000;
  
  // 分解为小时、分钟、秒
  uint32_t hours = totalSeconds / 3600;
  uint32_t minutes = (totalSeconds % 3600) / 60;
  uint32_t seconds = totalSeconds % 60;

  // 格式化输出（带前导零）
  Serial.print("历史累计时间: ");
  Serial.print(hours);
  Serial.print("小时 ");
  
  if (minutes < 10) Serial.print("0"); // 前导零
  Serial.print(minutes);
  Serial.print("分 ");
  
  if (seconds < 10) Serial.print("0"); // 前导零
  Serial.print(seconds);
  Serial.println("秒");
}

// 开始计时
void startTiming() {
  if (!isTiming) {
    isTiming = true;
    previousMillis = millis();  // 记录开始时间
    Serial.println("开始计时");
  }
}

// 停止计时
void stopTiming() {
  if (isTiming) {
    // 更新累计时间
    accumulatedMillis += (millis() - previousMillis);
    saveAccumulatedTime();  // 停止时保存一次，确保数据不丢失

    isTiming = false;
    Serial.println("停止计时");
    printAccumulatedTime();
  }
}

// 更新计时
void updateTiming() {
  if (isTiming) {
    unsigned long currentMillis = millis();

    // 每5分钟保存一次累计时间，减少Flash写入次数
    if (currentMillis - previousMillis >= 300000) {  // 5分钟 = 300000毫秒
      accumulatedMillis += 300000;
      previousMillis = currentMillis;
      saveAccumulatedTime();
      printAccumulatedTime();
    }
  }
}


