idf_component_register(SRCS "app_button.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES driver hall_wheel esp_timer button
                    )
