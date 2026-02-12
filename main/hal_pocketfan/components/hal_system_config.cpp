#include "../hal_pocketfan.h"
#include "esp_err.h"
#include "esp_log.h"
#include "nvs.h"
#include <string>

static const char *TAG = "hal_syscfg";

static constexpr const char *k_nvs_namespace = "syscfg";
static constexpr const char *k_key_brightness = "brightness";
static constexpr const char *k_key_orientation = "orient";
static constexpr const char *k_key_beep = "beep";
static constexpr const char *k_key_high_refresh = "hi_refresh";
static constexpr const char *k_key_reverse_encoder = "rev_enc";
static constexpr const char *k_key_invert = "invert";
static constexpr const char *k_key_dark_theme = "dark_theme";
static constexpr const char *k_key_locale = "locale";
static constexpr const char *k_key_current_offset = "curr_off";
static constexpr const char *k_key_wifi_ssid = "wifi_ssid";
static constexpr const char *k_key_wifi_pwd = "wifi_pwd";
static constexpr const char *k_key_startup_image = "startup";

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
    nvs_set_i32(handle, k_key_orientation, _config.orientation);
    nvs_set_u8(handle, k_key_beep, _config.beepOn ? 1 : 0);
    nvs_set_u8(handle, k_key_high_refresh, _config.highRefreshRate ? 1 : 0);
    nvs_set_u8(handle, k_key_reverse_encoder, _config.reverseEncoder ? 1 : 0);
    nvs_set_u8(handle, k_key_invert, _config.invertDisplay ? 1 : 0);
    nvs_set_u8(handle, k_key_dark_theme, _config.darkTheme ? 1 : 0);
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
