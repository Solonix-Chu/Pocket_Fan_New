/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "images.h"  // 注意是双引号，不是尖括号
#define IMAGES_H     //位图数组
#ifndef IMAGES_H     //引入位图数组
#endif               //引入位图数组

// 屏幕尺寸定义
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// 初始化OLED对象，指定I2C地址(0x3C是常见地址，若不显示可尝试0x3D)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// 1. 定义位图数组（将所有位图指针存入数组，顺序即播放顺序）
const uint8_t* animationFrames[] = {
  gImage_1, gImage_2, gImage_3, gImage_4, gImage_5,
  gImage_6, gImage_7, gImage_8, gImage_9, gImage_10,
  gImage_11, gImage_12, gImage_13, gImage_14, gImage_15,
  gImage_16, gImage_17, gImage_18, gImage_19, gImage_20
};

// 2. 动画状态变量（全局或类成员）
int currentFrame = 0;             // 当前播放的帧索引
unsigned long lastFrameTime = 0;  // 上一帧播放的时间
bool isPlaying = false;           // 动画是否正在播放（控制暂停/继续）
int fps = 40;                     //刷新率

void setup() {
  Serial.begin(115200);
  Wire.begin(33, 34);                           // 初始化I2C通信，指定SDA=GPIO8，SCL=GPIO9
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // 初始化OLED屏幕
  display.setRotation(0);                     //屏幕显示旋转180°，1=90°，2=180°
  display.clearDisplay();                     // 清屏
}




void loop() {
  display.clearDisplay();  //清屏
  GIFplay();               //执行内容缓存
  //  display.display();       //显示内容
}

// 4. 非阻塞播放函数（放在loop()中，高频执行）
void GIFplay() {
  // 检查是否到达帧间隔时间（替代delay()，非阻塞）
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime >= fps) {
    lastFrameTime = currentTime;  // 更新时间戳

    // 绘制当前帧
    // 绘制位图（显示位置：x=32, y=0, 宽=128, 高=64, 颜色=1）
    display.drawBitmap(0, 0, animationFrames[currentFrame], 128, 64, 1);
    display.display();  //显示内容
    // 切换到下一帧（播放完最后一帧后重置为0，循环播放）
    currentFrame++;
    if (currentFrame >= 20) {
      currentFrame = 0;
      // 若只需播放一次，可在此处添加 isPlaying = false;
    }
  }
}