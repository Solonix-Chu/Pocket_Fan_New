#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <string.h>
#include <u8g2.h>

#include "sdkconfig.h"
#include "u8g2_esp32_hal.h"
#include "usr_lcd.h"

// SDA - GPIO5
#define PIN_SDA 5

// SCL - GPIO6
#define PIN_SCL 6

static const char* TAG = "usr_lcd";

// Define the global u8g2 object
u8g2_t u8g2;

void usr_lcd_init(void)
{
  u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
  u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
  u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
  u8g2_esp32_hal_init(u8g2_esp32_hal);

  // Use the global u8g2 object
  u8g2_Setup_ssd1306_i2c_128x32_univision_f(
      &u8g2, U8G2_R0,
      u8g2_esp32_i2c_byte_cb,
      u8g2_esp32_gpio_and_delay_cb);
  u8x8_SetI2CAddress(&u8g2.u8x8, 0x78);

  ESP_LOGI(TAG, "Initializing display");
  u8g2_InitDisplay(&u8g2);
  u8g2_SetPowerSave(&u8g2, 0); 
  u8g2_ClearBuffer(&u8g2);
  u8g2_SendBuffer(&u8g2);
  ESP_LOGI(TAG, "Display initialized");
}