/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_lcd_panel_io_handle_t hal_display_get_io_handle(void);
esp_lcd_panel_handle_t hal_display_get_panel_handle(void);

#ifdef __cplusplus
}
#endif
