/*
#include <Wire.h>
// 定义I2C引脚（根据实际连接修改）
const int SDA_PIN = 0;
const int SCL_PIN = 1;
#define IP2366_I2C_ADDR 0x75
#define REG_POWER 0x74

// 读取8位寄存器（下方read16BitRegister用于16位寄存器读取并合并）
uint8_t read8BitRegister(uint8_t regAddr) {
  Wire.beginTransmission(IP2366_I2C_ADDR);            //启动传输
  Wire.write(regAddr);                                // 发送寄存器地址
  if (Wire.endTransmission(false) != 0) return 0xFF;  // 传输失败返回错误码

  if (Wire.requestFrom(IP2366_I2C_ADDR, 1) != 1) return 0xFF;
  return Wire.read();
}

// 读取16位寄存器（连续地址读取，优化时序）
int16_t read16BitRegister(uint8_t regAddr) {
  Wire.beginTransmission(IP2366_I2C_ADDR);
  Wire.write(regAddr);
  if (Wire.endTransmission(false) != 0) return -9999;           // 传输失败返回错误码
  if (Wire.requestFrom(IP2366_I2C_ADDR, 2) != 2) return -9998;  // 数据长度错误
  uint8_t lowByte = Wire.read();                                //先读低8位
  uint8_t highByte = Wire.read();                               //再读高8位
  return (highByte << 8) | lowByte;                             //合并为16位
}

void setup() {

  //串口初始化
  Serial.begin(115200);

  //I2C总线初始化
  Wire.begin(SDA_PIN, SCL_PIN);

  // 设置I2C时钟为200kHz
  Wire.setClock(200000);
  delay(500);  //开机图片显示持续时间
}

void loop() {
  // 读取功率（单位：10mW → W）
  int16_t power = read16BitRegister(REG_POWER);
  float sysPower = power * 0.01;
  Serial.print("系统功率: ");
  Serial.print(sysPower, 2);
  Serial.println(" W");
    delay(100);  //开机图片显示持续时间
}
*/