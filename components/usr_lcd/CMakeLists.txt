idf_component_register(SRCS "usr_lcd.c" "lvgl_demo_ui.c"
                    INCLUDE_DIRS "" "./include"
                    PRIV_REQUIRES esp_driver_gpio esp_timer esp_lcd lvgl esp_lvgl_port
                    )
