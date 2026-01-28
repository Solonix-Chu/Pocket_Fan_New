//页面一，读取2366并显示相关数据
void PAGE1() {
  u8g2.clearBuffer();//清屏
  u8g2.drawXBMP(0, 0, 128, 64, MAIN);  //坐标，分辨率，数组名字，写入主框图
  READ2366();//更新2366数据
  u8g2.sendBuffer();//开始显示
  //读取2366数据
  /*
  if (digitalRead(ENTER_PIN) == LOW) {
    delay(200);
    PAGE2();
  }
  else {
    PAGE1();  //在此页面循环
  }
  */
}