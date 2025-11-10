idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_driver_gpio esp_timer usr_lcd app_button u8g2 u8g2-hal-esp-idf
                       )
