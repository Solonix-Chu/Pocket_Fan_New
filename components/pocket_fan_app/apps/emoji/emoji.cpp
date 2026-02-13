#include "emoji.h"
#include "../../assets/assets.h"
#include "../../hal/hal.h"
#include "../apps.h"
#include <esp_log.h>
#include <cstdio>

namespace {
constexpr int k_default_screensaver_timeout_sec = 5;
constexpr int k_timeout_options[] = {0, 5, 10, 30};
constexpr int k_item_timeout = 0;
constexpr int k_timeout_option_min = 0;
constexpr int k_timeout_option_max = static_cast<int>(sizeof(k_timeout_options) / sizeof(k_timeout_options[0])) - 1;
} // namespace

static const char* TAG = "EmojiApp";

void EmojiApp::onCreate()
{
    setAppInfo().name = "Emoji";
}

void EmojiApp::onOpen()
{
    auto& config = HAL::GetSystemConfig();
    if (!_isValidTimeout(config.screensaverTimeoutSec)) {
        config.screensaverTimeoutSec = k_default_screensaver_timeout_sec;
        HAL::SaveSystemConfig();
    }

    _view = new SettingsView();
    _view->addSettingsItem({_formatTimeoutItemText(), false, false, [this]() {
        auto& cfg = HAL::GetSystemConfig();
        const int next = _adjustTimeout(cfg.screensaverTimeoutSec, +1);
        if (next != cfg.screensaverTimeoutSec) {
            cfg.screensaverTimeoutSec = next;
            HAL::SaveSystemConfig();
            ESP_LOGI(TAG, "ScreenSaver timeout set: %d sec", cfg.screensaverTimeoutSec);
        }
        if (_view) {
            _view->updateItemText(k_item_timeout, _formatTimeoutItemText());
        }
    }});
    _view->addSettingsItem({AssetPool::GetText().AppSettings_Option_Back, false, false, [this]() {
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    _view->init();
}

void EmojiApp::onRunning()
{
    if (!_view) {
        return;
    }

    _view->update();
}

void EmojiApp::onClose()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}

void EmojiApp::onDestroy()
{
    onClose();
}

std::string EmojiApp::_formatTimeoutItemText() const
{
    const auto& tr = AssetPool::GetText();
    const auto& config = HAL::GetSystemConfig();
    char value_buf[16] = {};
    if (config.screensaverTimeoutSec == 0) {
        snprintf(value_buf, sizeof(value_buf), "%s", tr.AppSettings_Option_Off);
    } else {
        snprintf(value_buf, sizeof(value_buf), "%dS", config.screensaverTimeoutSec);
    }

    char line_buf[48] = {};
    snprintf(line_buf, sizeof(line_buf), tr.PocketFan_Settings_ScreenSaverTimeoutFmt, value_buf);
    return line_buf;
}

bool EmojiApp::_isValidTimeout(int timeout_sec) const
{
    for (int option : k_timeout_options) {
        if (option == timeout_sec) {
            return true;
        }
    }
    return false;
}

int EmojiApp::_timeoutOptionIndex(int timeout_sec) const
{
    for (size_t i = 0; i < sizeof(k_timeout_options) / sizeof(k_timeout_options[0]); i++) {
        if (k_timeout_options[i] == timeout_sec) {
            return static_cast<int>(i);
        }
    }
    return 1; // default 5s index
}

int EmojiApp::_adjustTimeout(int timeout_sec, int delta) const
{
    int idx = _timeoutOptionIndex(timeout_sec);
    idx += delta;
    const int option_count = k_timeout_option_max - k_timeout_option_min + 1;
    while (idx < k_timeout_option_min) {
        idx += option_count;
    }
    while (idx > k_timeout_option_max) {
        idx -= option_count;
    }
    return k_timeout_options[idx];
}
