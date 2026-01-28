void PAGE_CHANGE() {
  if (PAGE == 0) {
    emotion = random(min_value, max_value);  //随机数更新
    Display_Face();                          //执行Dsplay_Face函数，带着emotion赋值执行一次表情切换
    Serial.println("这里是页面0");
    PAGE0();  //切换到第0页面
  } else if (PAGE == 1) {
    PAGE1();//切换到第1页面
  } else if (PAGE == 2) {
    PAGE2();//切换到第2页面
  } else if (PAGE == 3) {
    PAGE3();//切换到第3页面
  } else if (PAGE == 4) {
    PAGE4();//切换到第4页面
  } 
}


    //u8g2.clearBuffer();
    // u8g2.drawXBMP(47, 19, 34, 26, PAGE2_ICON);  //坐标，分辨率，数组名字
    // u8g2.sendBuffer();
