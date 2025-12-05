idf_component_register(SRCS "main.cpp" 
                       "hal_pocketfan/components/hal_display.cpp"
                       "hal_pocketfan/components/hal_lvgl.cpp"
                       "hal_pocketfan/components/hal_button.cpp"
                       "hal_pocketfan/components/hal_watch_dog.cpp"
                       INCLUDE_DIRS "." "hal_pocketfan"
                       PRIV_REQUIRES esp_driver_gpio esp_timer LovyanGFX app_button pocket_fan_app mooncake
                       )
