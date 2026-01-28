/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
// #include "app_base_test/app_base_test.h"
#include "launcher/launcher.h"
#include "homepage/homepage.h"
#include "settings/settings.h"
// #include "app_power_monitor/app_power_monitor.h"
// #include "app_template/app_template.h"
// #include "app_waveform/app_waveform.h"
// #include "settings/settings.h"
// #include "app_hal_test/app_hal_test.h"
// #include "app_files/app_files.h"
// #include "app_remote/app_remote.h"
#include "app_startup_anim/app_startup_anim.h"
#include "hardware_test/hardware_test.h"
#include "enjoy/enjoy.h"
#include "health/health.h"
/* Header files locator (Don't remove) */

namespace APPS {
    extern int homepage_id;
    extern int menu_id;
    extern int settings_id;
    extern int enjoy_id;
    extern int health_id;
}

/**
 * @brief Run startup anim app
 *
 * @param mooncake
 */
inline void app_run_startup_anim(mooncake::Mooncake* mooncake)
{
    int id = mooncake->installApp(std::make_unique<AppStartupAnim>());
    mooncake->openApp(id);

    // while (1)
    // {
    //     mooncake->update();
    //     if (mooncake->getAppCurrentState(id) == mooncake::AppAbility::StateSleeping)
    //         break;
    // }

    mooncake->uninstallApp(id);
}

/**
 * @brief Install launcher
 *
 * @param mooncake
 */
// inline void app_install_launcher(MOONCAKE::Mooncake* mooncake)
// {
//     auto launcher = new MOONCAKE::APPS::AppLauncher_Packer;
//     // auto launcher = new MOONCAKE::APPS::AppWaveform_Packer;
//     mooncake->installApp(launcher);
//     mooncake->createAndStartApp(launcher);
// }

/**
 * @brief Install apps
 *
 * @param mooncake
 */
// inline void app_install_apps(MOONCAKE::Mooncake* mooncake)
// {
//     // mooncake->installApp(new MOONCAKE::APPS::AppTemplate_Packer);
//     mooncake->installApp(new MOONCAKE::APPS::AppPower_monitor_Packer);
//     mooncake->installApp(new MOONCAKE::APPS::AppWaveform_Packer);
//     mooncake->installApp(new MOONCAKE::APPS::AppFiles_Packer);
//     // mooncake->installApp(new MOONCAKE::APPS::AppRemote_Packer);
//     mooncake->installApp(new MOONCAKE::APPS::AppSettings_Packer);
//     // mooncake->installApp(new MOONCAKE::APPS::AppBase_test_Packer);
//     // mooncake->installApp(new MOONCAKE::APPS::AppHal_test_Packer);
//     /* Install app locator (Don't remove) */
// }
