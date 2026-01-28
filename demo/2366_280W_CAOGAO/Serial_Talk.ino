// 此页处理串口接收到的命令，不区分大小写


void processCommand() {
  // 转换为小写以便不区分大小写
  for (int i = 0; commandBuffer[i]; i++) {
    commandBuffer[i] = tolower(commandBuffer[i]);
  }


  if (strcmp(commandBuffer, "clear") == 0) {  // 检查是否是clear命令,当收到“clear”命令时，则重置计时器。
    Serial.println("接收到清除命令，正在重置累计时间...");
    clear();  //执行重置计时器函数
    Serial.println("累计时间已重置为0,3秒后代码继续运行");
    delay(3000);
  } else if (strcmp(commandBuffer, "menu") == 0) {
    Serial.println("命令查询=====menu");
    Serial.println("重置计时器=====clear");
    delay(3000);
  } else {
    Serial.print("未知命令: ");
    Serial.println(commandBuffer);
    Serial.println("命令查询请输入：“menu”");
    delay(3000);
  }
}