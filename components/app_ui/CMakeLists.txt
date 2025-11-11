set(srcs "app_ui.c"
         "ui_assets.c"
         "ui_pages.c"
         "ui_utils.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "include"
                       PRIV_INCLUDE_DIRS "."
                       REQUIRES app_button usr_lcd esp_timer u8g2 button
                       )