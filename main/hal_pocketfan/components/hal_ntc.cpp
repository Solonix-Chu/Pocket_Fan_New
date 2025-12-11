/*
 * SPDX-FileCopyrightText: 2024 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "../hal_pocketfan.h"
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <esp_log.h>
#include <math.h>

static const char *TAG = "HAL_NTC";

static adc_oneshot_unit_handle_t _adc1_handle = NULL;
static adc_oneshot_unit_handle_t _adc2_handle = NULL;
static adc_cali_handle_t _adc1_cali_handle = NULL;
static adc_cali_handle_t _adc2_cali_handle = NULL;

void HAL_PocketFan::_ntc_init()
{
    ESP_LOGI(TAG, "NTC Init");

    // ADC1 Init (GPIO 6 - CH5 on S3)
    // NOTE: GPIO 6 is often SPI CLK! Ensure this is safe on your board.
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    if (adc_oneshot_new_unit(&init_config1, &_adc1_handle) == ESP_OK) {
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_oneshot_config_channel(_adc1_handle, ADC_CHANNEL_5, &config);

        // Calibration
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_1,
            .chan = ADC_CHANNEL_5,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_cali_create_scheme_curve_fitting(&cali_config, &_adc1_cali_handle);
    } else {
        ESP_LOGE(TAG, "ADC1 Init Failed (GPIO 6)");
    }

    // ADC2 Init (GPIO 12 - CH1 on S3)
    adc_oneshot_unit_init_cfg_t init_config2 = {
        .unit_id = ADC_UNIT_2,
        .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    if (adc_oneshot_new_unit(&init_config2, &_adc2_handle) == ESP_OK) {
        adc_oneshot_chan_cfg_t config = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_oneshot_config_channel(_adc2_handle, ADC_CHANNEL_1, &config);

        // Calibration
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = ADC_UNIT_2,
            .chan = ADC_CHANNEL_1,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        adc_cali_create_scheme_curve_fitting(&cali_config, &_adc2_cali_handle);
    } else {
        ESP_LOGE(TAG, "ADC2 Init Failed (GPIO 12)");
    }
}

float HAL_PocketFan::getNTC(int channel)
{
    int adc_raw = 0;
    int voltage = 0;
    esp_err_t ret;
    
    // 1. 读取 ADC 电压
    if (channel == 0 && _adc1_handle) { // GPIO 6 (电机温度)
        ret = adc_oneshot_read(_adc1_handle, ADC_CHANNEL_5, &adc_raw);
        if (ret == ESP_OK) {
            if (_adc1_cali_handle) {
                adc_cali_raw_to_voltage(_adc1_cali_handle, adc_raw, &voltage);
            } else {
                voltage = adc_raw * 3300 / 4095;
            }
        }
    } else if (channel == 1 && _adc2_handle) { // GPIO 12 (板载温度)
        ret = adc_oneshot_read(_adc2_handle, ADC_CHANNEL_1, &adc_raw);
        if (ret == ESP_OK) {
            if (_adc2_cali_handle) {
                adc_cali_raw_to_voltage(_adc2_cali_handle, adc_raw, &voltage);
            } else {
                voltage = adc_raw * 3300 / 4095;
            }
        }
    } else {
        return -999.0f; // 无效通道
    }

    // 2. 简单的断路/短路保护 (电压接近 0 或 3.3V 视为异常)
    if (voltage >= 3250 || voltage <= 50) return -999.0f;

    // 3. 定义该通道对应的 NTC 参数
    float R_pullup = 100000.0f; // 原理图 R41 和 R53 都是 100k
    float R_ntc_nominal = 0.0f; // 25度时的阻值
    float Beta = 0.0f;          // B值

    if (channel == 0) {
        // --- GPIO 6 (电机) ---
        // 原理图 U34: B=3380, R=10K
        R_ntc_nominal = 10000.0f;
        Beta = 3380.0f;
    } else {
        // --- GPIO 12 (板载) ---
        // 原理图 R59: NTC 4200B 100K (即 B=4200, R=100K)
        R_ntc_nominal = 100000.0f;
        Beta = 4200.0f;
    }

    // 4. 计算当前 NTC 阻值
    // 原理图显示 NTC 接地 (GND)，上拉电阻 (100k) 接 3V3
    // 分压公式: V_adc = 3300 * R_ntc / (R_pullup + R_ntc)
    // 反推 R_ntc: R_ntc = (V_adc * R_pullup) / (3300 - V_adc)
    float r_ntc_current = ((float)voltage * R_pullup) / (3300.0f - (float)voltage);
    
    // 5. 使用 Steinhart-Hart (Beta) 公式计算温度
    // T = 1 / (1/T0 + 1/B * ln(R/R0))
    // T0 = 25°C = 298.15K
    float t_kelvin = 1.0f / (1.0f/298.15f + (1.0f/Beta) * log(r_ntc_current / R_ntc_nominal));
    
    return t_kelvin - 273.15f; // 转换为摄氏度
}