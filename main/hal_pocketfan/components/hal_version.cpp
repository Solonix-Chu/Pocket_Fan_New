/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <esp_app_desc.h>

std::string HAL_PocketFan::version()
{
    const esp_app_desc_t* desc = esp_app_get_description();
    if (desc && desc->version[0] != '\0') {
        return std::string(desc->version);
    }
    return APP_VERSION;
}

