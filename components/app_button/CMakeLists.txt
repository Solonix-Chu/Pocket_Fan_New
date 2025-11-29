idf_component_register(SRCS "app_button.c"
                    INCLUDE_DIRS "include"
                    REQUIRES button
                    PRIV_REQUIRES driver hall_wheel esp_timer
                    )
