void key() {


  if (digitalRead(KEY_PIN) == LOW) {  //若2366提供低电平，且唤醒超过10s，才进入休眠

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("GJZ......");
    display.display();
    delay(1000);
    if (digitalRead(KEY_PIN) == LOW) {

      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("GJCG");
      display.display();
      delay(1000);
      display.clearDisplay();
      display.display();
      digitalWrite(WAKE_PIN, LOW);
      delay(300);
      digitalWrite(EN_PIN, LOW);
      digitalWrite(ESC_PIN, LOW);
    } else {
      digitalWrite(EN_PIN, HIGH);
      display.clearDisplay();
      display.setCursor(0, 0);
      display.println("GO LOOP");
      display.display();
      delay(1000);
    }
  }

  /*

  if (digitalRead(UP_PIN) == LOW) { digitalWrite(35, LOW); }
  if (digitalRead(UP_PIN) == HIGH) { digitalWrite(35, HIGH); }

  if (digitalRead(DOWN_PIN) == LOW) { digitalWrite(36, LOW); }
  if (digitalRead(DOWN_PIN) == HIGH) { digitalWrite(36, HIGH); }

  if (digitalRead(LEFT_PIN) == LOW) { digitalWrite(37, LOW); }
  if (digitalRead(LEFT_PIN) == HIGH) { digitalWrite(37, HIGH); }

  if (digitalRead(RIGHT_PIN) == LOW) { digitalWrite(38, LOW); }
  if (digitalRead(RIGHT_PIN) == HIGH) { digitalWrite(38, HIGH); }
  */
}