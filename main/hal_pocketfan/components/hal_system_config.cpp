#include "../hal_pocketfan.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <cmath>
#include <cstdint>
#include <limits>
#include <string>

static const char *TAG = "hal_syscfg";

static constexpr const char *k_nvs_namespace = "syscfg";
static constexpr const char *k_key_brightness = "brightness";
static constexpr const char *k_key_screensaver_timeout = "ss_to";
static constexpr const char *k_key_ui_anim_speed = "ui_spd";
static constexpr const char *k_key_orientation = "orient";
static constexpr const char *k_key_beep = "beep";
static constexpr const char *k_key_high_refresh = "hi_refresh";
static constexpr const char *k_key_reverse_encoder = "rev_enc";
static constexpr const char *k_key_invert = "invert";
static constexpr const char *k_key_dark_theme = "dark_theme";
static constexpr const char *k_key_bluetooth_enabled = "bt_en";
static constexpr const char *k_key_trackball_mode = "tb_mode";
static constexpr const char *k_key_trackball_r = "tb_r";
static constexpr const char *k_key_trackball_g = "tb_g";
static constexpr const char *k_key_trackball_b = "tb_b";
static constexpr const char *k_key_trackball_w = "tb_w";
static constexpr const char *k_key_locale = "locale";
static constexpr const char *k_key_current_offset = "curr_off";
static constexpr const char *k_key_wifi_ssid = "wifi_ssid";
static constexpr const char *k_key_wifi_pwd = "wifi_pwd";
static constexpr const char *k_key_startup_image = "startup";

// Keep health stats across factory reset.
static constexpr const char *k_pm_namespace = "pm";
static constexpr const char *k_pm_key_mah10 = "mah10";
static constexpr const char *k_pm_key_cycles = "cycles";
static constexpr const char *k_pm_key_mtrh10 = "mtrh10";

static bool nvs_read_str(nvs_handle_t handle, const char *key, std::string *out)
{
    size_t len = 0;
    esp_err_t err = nvs_get_str(handle, key, nullptr, &len);
    if (err != ESP_OK || len == 0) {
        return false;
    }

    std::string buf;
    buf.resize(len);
    err = nvs_get_str(handle, key, buf.data(), &len);
    if (err != ESP_OK) {
        return false;
    }

    if (len > 0 && buf[len - 1] == '\0') {
        buf.resize(len - 1);
    }
    *out = buf;
    return true;
}

static uint32_t to_u32_rounded_nonneg(float value)
{
    constexpr uint32_t kU32Max = std::numeric_limits<uint32_t>::max();
    if (!std::isfinite(value) || value <= 0.0f) {
        return 0;
    }
    const float rounded = value + 0.5f;
    if (rounded >= static_cast<float>(kU32Max)) {
        return kU32Max;
    }
    return static_cast<uint32_t>(rounded);
}

static esp_err_t restore_health_snapshot_to_nvs(float discharged_mah, float cycles, float motor_hours)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(k_pm_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        return err;
    }

    const uint32_t mah10 = to_u32_rounded_nonneg(discharged_mah * 10.0f);
    const uint32_t cyc = to_u32_rounded_nonneg(cycles);
    const uint32_t mtrh10 = to_u32_rounded_nonneg(motor_hours * 10.0f);

    err = nvs_set_u32(handle, k_pm_key_mah10, mah10);
    if (err == ESP_OK) err = nvs_set_u32(handle, k_pm_key_cycles, cyc);
    if (err == ESP_OK) err = nvs_set_u32(handle, k_pm_key_mtrh10, mtrh10);
    if (err == ESP_OK) err = nvs_commit(handle);

    nvs_close(handle);
    return err;
}

void HAL_PocketFan::loadSystemConfig()
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(k_nvs_namespace, NVS_READONLY, &handle);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "nvs_open(read) failed: %s", esp_err_to_name(err));
        return;
    }

    int32_t i32 = 0;
    if (nvs_get_i32(handle, k_key_brightness, &i32) == ESP_OK) {
        _config.brightness = i32;
    }
    if (nvs_get_i32(handle, k_key_screensaver_timeout, &i32) == ESP_OK) {
        if (i32 == 0 || i32 == 5 || i32 == 10 || i32 == 30) {
            _config.screensaverTimeoutSec = i32;
        }
    }
    if (nvs_get_i32(handle, k_key_ui_anim_speed, &i32) == ESP_OK) {
        if (i32 >= 50 && i32 <= 200) {
            _config.uiAnimSpeedPct = i32;
        }
    }
    if (nvs_get_i32(handle, k_key_orientation, &i32) == ESP_OK) {
        _config.orientation = i32;
    }

    uint8_t u8 = 0;
    if (nvs_get_u8(handle, k_key_beep, &u8) == ESP_OK) {
        _config.beepOn = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_high_refresh, &u8) == ESP_OK) {
        _config.highRefreshRate = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_reverse_encoder, &u8) == ESP_OK) {
        _config.reverseEncoder = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_invert, &u8) == ESP_OK) {
        _config.invertDisplay = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_dark_theme, &u8) == ESP_OK) {
        _config.darkTheme = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_bluetooth_enabled, &u8) == ESP_OK) {
        _config.bluetoothEnabled = (u8 != 0);
    }
    if (nvs_get_u8(handle, k_key_trackball_mode, &u8) == ESP_OK) {
        if (u8 <= CONFIG::TRACKBALL_LED_CUSTOM_STATIC) {
            _config.trackballLedMode = static_cast<CONFIG::TrackballLedMode_t>(u8);
        }
    }
    if (nvs_get_u8(handle, k_key_trackball_r, &u8) == ESP_OK) {
        _config.trackballR = u8;
    }
    if (nvs_get_u8(handle, k_key_trackball_g, &u8) == ESP_OK) {
        _config.trackballG = u8;
    }
    if (nvs_get_u8(handle, k_key_trackball_b, &u8) == ESP_OK) {
        _config.trackballB = u8;
    }
    if (nvs_get_u8(handle, k_key_trackball_w, &u8) == ESP_OK) {
        _config.trackballW = u8;
    }
    if (nvs_get_u8(handle, k_key_locale, &u8) == ESP_OK) {
        if (u8 <= locale_code_cn) {
            _config.localeCode = static_cast<LocaleCode_t>(u8);
        }
    }

    float current_offset = 0.0f;
    size_t float_size = sizeof(current_offset);
    if (nvs_get_blob(handle, k_key_current_offset, &current_offset, &float_size) == ESP_OK &&
        float_size == sizeof(current_offset)) {
        _config.currentOffset = current_offset;
    }

    std::string value;
    if (nvs_read_str(handle, k_key_wifi_ssid, &value)) {
        _config.wifiSsid = value;
    }
    if (nvs_read_str(handle, k_key_wifi_pwd, &value)) {
        _config.wifiPassword = value;
    }
    if (nvs_read_str(handle, k_key_startup_image, &value)) {
        _config.startupImage = value;
    }

    nvs_close(handle);
}

void HAL_PocketFan::saveSystemConfig()
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(k_nvs_namespace, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open(write) failed: %s", esp_err_to_name(err));
        return;
    }

    nvs_set_i32(handle, k_key_brightness, _config.brightness);
    nvs_set_i32(handle, k_key_screensaver_timeout, _config.screensaverTimeoutSec);
    nvs_set_i32(handle, k_key_ui_anim_speed, _config.uiAnimSpeedPct);
    nvs_set_i32(handle, k_key_orientation, _config.orientation);
    nvs_set_u8(handle, k_key_beep, _config.beepOn ? 1 : 0);
    nvs_set_u8(handle, k_key_high_refresh, _config.highRefreshRate ? 1 : 0);
    nvs_set_u8(handle, k_key_reverse_encoder, _config.reverseEncoder ? 1 : 0);
    nvs_set_u8(handle, k_key_invert, _config.invertDisplay ? 1 : 0);
    nvs_set_u8(handle, k_key_dark_theme, _config.darkTheme ? 1 : 0);
    nvs_set_u8(handle, k_key_bluetooth_enabled, _config.bluetoothEnabled ? 1 : 0);
    nvs_set_u8(handle, k_key_trackball_mode, static_cast<uint8_t>(_config.trackballLedMode));
    nvs_set_u8(handle, k_key_trackball_r, _config.trackballR);
    nvs_set_u8(handle, k_key_trackball_g, _config.trackballG);
    nvs_set_u8(handle, k_key_trackball_b, _config.trackballB);
    nvs_set_u8(handle, k_key_trackball_w, _config.trackballW);
    nvs_set_u8(handle, k_key_locale, static_cast<uint8_t>(_config.localeCode));
    nvs_set_blob(handle, k_key_current_offset, &_config.currentOffset, sizeof(_config.currentOffset));

    if (!_config.wifiSsid.empty()) {
        nvs_set_str(handle, k_key_wifi_ssid, _config.wifiSsid.c_str());
    } else {
        nvs_erase_key(handle, k_key_wifi_ssid);
    }

    if (!_config.wifiPassword.empty()) {
        nvs_set_str(handle, k_key_wifi_pwd, _config.wifiPassword.c_str());
    } else {
        nvs_erase_key(handle, k_key_wifi_pwd);
    }

    if (!_config.startupImage.empty()) {
        nvs_set_str(handle, k_key_startup_image, _config.startupImage.c_str());
    } else {
        nvs_erase_key(handle, k_key_startup_image);
    }

    err = nvs_commit(handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_commit failed: %s", esp_err_to_name(err));
    }
    nvs_close(handle);
}

void HAL_PocketFan::applySystemConfig()
{
    int brightness = _config.brightness;
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    setDisplayBrightness(static_cast<uint8_t>(brightness));
    setDisplayInvert(_config.invertDisplay);
}

void HAL_PocketFan::factoryReset(OnLogPageRenderCallback_t onLogPageRender)
{
    if (onLogPageRender) onLogPageRender("Factory reset: stopping subsystems...");

    // Snapshot health stats so factory reset won't clear them.
    const float health_discharged_mah = getBatteryDischargedMah();
    const float health_cycles = getBatteryCycles();
    const float health_motor_hours = getMotorHours();

    // Best-effort cleanup before erase.
    (void)stopBleOta();
    setFanSpeed(0.0f);
    setFanState(false);
    setLedBreath(false);
    setLed(0, 0, 0, 0);

    if (onLogPageRender) onLogPageRender("Factory reset: erasing NVS...");
    esp_err_t err = nvs_flash_erase();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_flash_erase failed: %s", esp_err_to_name(err));
        if (onLogPageRender) onLogPageRender("Factory reset: NVS erase failed");
    } else {
        if (onLogPageRender) onLogPageRender("Factory reset: restoring health data...");

        err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            // Should be rare after erase, but handle it defensively.
            (void)nvs_flash_erase();
            err = nvs_flash_init();
        }

        if (err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_flash_init failed after erase: %s", esp_err_to_name(err));
            if (onLogPageRender) onLogPageRender("Factory reset: NVS init failed");
        } else {
            err = restore_health_snapshot_to_nvs(
                health_discharged_mah,
                health_cycles,
                health_motor_hours);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "restore health data failed: %s", esp_err_to_name(err));
                if (onLogPageRender) onLogPageRender("Factory reset: health restore failed");
            } else if (onLogPageRender) {
                onLogPageRender("Factory reset: done, rebooting...");
            }
        }
    }

    // Give logs a moment to flush.
    vTaskDelay(pdMS_TO_TICKS(100));
    esp_restart();
}
