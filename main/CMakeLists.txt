idf_component_register(SRCS "main.cpp" 
                       "hal_pocketfan/components/hal_display.cpp"
                       "hal_pocketfan/components/hal_lvgl.cpp"
                       "hal_pocketfan/components/hal_button.cpp"
                       "hal_pocketfan/components/hal_watch_dog.cpp"
                       "hal_pocketfan/components/hal_power.cpp"
                       "hal_pocketfan/components/hal_power_monitor.cpp"
                       "hal_pocketfan/components/hal_ntc.cpp"
                       "hal_pocketfan/components/hal_motor.cpp"
                       "hal_pocketfan/components/hal_led.cpp"
                       INCLUDE_DIRS "." "hal_pocketfan"
                       PRIV_REQUIRES esp_driver_gpio esp_timer LovyanGFX app_button pocket_fan_app mooncake esp_adc
                       )
