/*
PWM分辨率：
1 位：2 级（0~1）
8 位：256 级（0~255）
10 位：1024 级（0~1023）
11 位：2048 级（0~2047）
12 位：4096 级（0~4095）
16 位：65536 级（0~65535）

PWM 输出频率范围：
分辨率（位数）	  最大理论频率	                  实用频率范围（推荐）	             典型应用场景
1  位	           40MHz	                        1kHz ~ 40MHz                    高速脉冲控制
8  位	           312.5kHz	                      1kHz ~ 200kHz	                  普通 LED 亮度调节
10 位            78.125kHz	                    1kHz ~ 50kHz	                  无频闪 LED 照明（高频）
12 位	           19.531kHz	                    1kHz ~ 15kHz	                  精细亮度调节
16 位	           1.22kHz	                      10Hz ~ 1kHz                     低频模拟信号输出

GPIO1：UP
GPIO2：DOWN
GPIO3：LEFT
GPIO4：RIGHT
GPIO5：PWM OUT
GPIO6：电机温度检测
GPIO7：暴力按键
GPIO10：电调MOS开关
GPIO12：板载温度检测
GPIO18：IIC INT（IP2369）
GPIO19：USB DN
GPIO20：USB DP
GPIO33：IIC SDA
GPIO34：IIC SCL
GPIO35：LED W
GPIO36：LED R
GPIO37：LED G
GPIO38：LED B
GPIO45：供电使能
GPIO46：开关机+功能按键
*/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_INA219.h>

// 屏幕尺寸定义
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64


// 初始化OLED对象，指定I2C地址(0x3C是常见地址，若不显示可尝试0x3D)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 创建INA219对象
Adafruit_INA219 ina219;
// 校准参数配置（根据INA219模块规格选择）
#define CALIBRATION_32V_2A    // 32V/2A规格模块启用此宏


// 引脚定义（按需求匹配）
#define EN_PIN 45   // 供电使能
#define KEY_PIN 46  // 开关机和功能按键
#define ESC_PIN 10  // 开关机和功能按键
#define UP_PIN 1
#define DOWN_PIN 2
#define LEFT_PIN 3
#define RIGHT_PIN 4
#define WAKE_PIN 18  // 唤醒引脚，此处连接IP2366的int引脚
// 定义I2C引脚（根据实际连接修改）
const int SDA_PIN = 33;
const int SCL_PIN = 34;


//【2366相关全局变量】============================================
// 【IP2366】
#define IP2366_I2C_ADDR 0x75  // 7位从机地址（非8位传输地址）【手册地址是0xEA，但在 Arduino 的wire库中，发送从机地址时会自动处理左移，而0x75左移一位就是0xEA】
// 寄存器地址定义（根据文档V1.13）
#define REG_BAT_VOLT 0x50        // 电池电压（低8位+高8位连续地址）
#define REG_SYS_VOLT 0x52        // 系统电压
#define REG_BAT_CURR 0x6E        // 电池电流
#define REG_SYS_CURR 0x70        // 系统电流
#define REG_POWER 0x74           // 功率（10mW单位）
#define REG_CHARGE_STATUS1 0x31  // 充电状态（8位状态寄存器）
#define REG_CHARGE_STATUS2 0x32  // 充电状态（8位状态寄存器）
#define REG_BAT_CAPACITY 0x40    // 电池剩余容量（mAh）
#define REG_FULL_CAPACITY 0x42   // 电池满电容量（mAh）
#define REG_CHARGE_LIMIT 0x44    // 充电电流限制（mA，有符号数）
#define REG_DISCHG_CUTOFF 0x46   // 放电截止电压（mV）
#define REG_DEVICE_STATUS 0x34   // 输入输出状态以及协议读取（8位）
#define REG_CYCLE_COUNT 0x4A     // 充放电循环次数
#define REG_TYPEC_MODE 0x22      // TypeC模式读取  UFP接收电源  DFP提供电源  DRP自动识别[测试]
#define REG_TEMP_CURR 0x77       // NTC输出电流控制寄存器
#define REG_TEMP_VOLT_LOW 0x78   // NTC电压数据
#define REG_TIMENODE1 0x69       //时间戳寄存器第1位
#define REG_TIMENODE2 0x6A       //时间戳寄存器第2位
#define REG_TIMENODE3 0x6B       //时间戳寄存器第3位
#define REG_TIMENODE4 0x6C       //时间戳寄存器第4位
#define REG_TIMENODE5 0x6D       //时间戳寄存器第5位

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
// NTC参数配置（根据实际NTC型号修改）
const float B_VALUE = 3380.0;        // NTC的B值（常见值为3950K）【const float，定义不可修改的浮点数变量，即支持小数，在代码中不可重新赋值】
const float R_REF = 10000.0;         // 参考电阻值(Ω)
const float R_ROOM = 10000.0;        // 室温(25°C)下的电阻值(Ω)
const float T_ROOM = 25.0 + 273.15;  // 室温(25°C)转换为开尔文温度

// 将电压值转换为温度值（使用Steinhart-Hart方程）2366温度读取
float convertVoltageToTemperature(float NTCvoltage, float ntcCurrent) {
  // 计算NTC电阻值
  float NTCresistance = NTCvoltage / ntcCurrent;

  // 使用Steinhart-Hart方程计算温度（更准确的非线性转换）
  float steinhart;
  steinhart = NTCresistance / R_REF;  // (R/Rref)
  steinhart = log(steinhart);         // ln(R/Rref)
  steinhart /= B_VALUE;               // 1/B * ln(R/Rref)
  steinhart += 1.0 / T_ROOM;          // + (1/T0)
  steinhart = 1.0 / steinhart;        // 取倒数

  return steinhart - 273.15;  // 转换为摄氏度
}
//读取温度数据
// 读取NTC输出电流设置
byte ntcIADCData = read8BitRegister(REG_TEMP_CURR);
float ntcCurrent = (ntcIADCData & 0x80) ? 80.0e-6 : 20.0e-6;  // A，根据bit7选择80uA或20uA【float，定义可修改的浮点数变量，即支持小数，在代码中可重新赋值】

// 读取NTC电压
uint16_t vNTC = read16BitRegister(REG_TEMP_VOLT_LOW);  //【读取 16 位 ADC（模拟转数字）值】
float NTCvoltage = vNTC * 3.3 / 4095.0;                // 假设ADC为12位（根据手册实际分辨率调整）

// 将NTC电压转换为温度（使用Steinhart-Hart方程）
float temperature = convertVoltageToTemperature(NTCvoltage, ntcCurrent);

// 电量百分比，定义映射区间（4串锂电池：12.0V→0%，16.8V→100%）
float VOLTAGE_MIN = 12.0;  // 映射起点（0%）
float VOLTAGE_MAX = 16.8;  // 映射终点（100%）

//INA219变量定义
float shuntvoltage = 0;
float busvoltage = 0;
float current_mA = 0;
float power_mW = 0;
float V = 0;
float C = 0;  //单位换算，mA换算成A，并保留两位小数(原适用于0.1Ω电阻，所以需要除以1000，但需要测更大功率，所以更换了0.01Ω电阻，所以需要放大十倍，所以此处只除以100)
float P = 0;  //单位换算，mW换算成W，并保留两位小数
