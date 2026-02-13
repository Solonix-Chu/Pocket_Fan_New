#include "settings.h"
#include "../../hal/hal.h"
#include "../../assets/assets.h"
#include "../apps.h"
#include <esp_log.h>
#include <esp_system.h>
#include <cstdio>

static const char* TAG = "SettingsApp";

namespace {
void set_random_trackball_targets()
{
    auto rand_u8 = [](uint8_t max_val) -> uint8_t {
        return static_cast<uint8_t>(esp_random() % (max_val + 1));
    };

    uint8_t r = rand_u8(180);
    uint8_t g = rand_u8(180);
    uint8_t b = rand_u8(180);
    uint8_t w = rand_u8(120);
    if ((r | g | b | w) == 0) {
        g = 120;
    }
    HAL::SetLed(r, g, b, w);
}

void apply_trackball_effect_preview(int mode, int r, int g, int b, int w)
{
    if (mode == CONFIG::TRACKBALL_LED_CUSTOM_STATIC) {
        HAL::SetLedBreath(false);
        HAL::SetLed(static_cast<uint8_t>(r),
                    static_cast<uint8_t>(g),
                    static_cast<uint8_t>(b),
                    static_cast<uint8_t>(w));
    } else {
        HAL::SetLedBreath(true);
        set_random_trackball_targets();
    }
}
} // namespace

void SettingsApp::onCreate()
{
    setAppInfo().name = "Settings";
}

void SettingsApp::onOpen()
{
    ESP_LOGI(TAG, "onOpen");
    _create_view();

    // Ensure Settings uses normal configurable LED effect (not menu-only green).
    const auto& cfg = HAL::GetSystemConfig();
    apply_trackball_effect_preview(static_cast<int>(cfg.trackballLedMode),
                                   static_cast<int>(cfg.trackballR),
                                   static_cast<int>(cfg.trackballG),
                                   static_cast<int>(cfg.trackballB),
                                   static_cast<int>(cfg.trackballW));
}

void SettingsApp::onRunning()
{
    const uint32_t now = HAL::Millis();

    if (_popup_mode != PopupMode::None) {
        auto btn_right = HAL::GetButton(BUTTON::BTN_RIGHT);
        auto btn_left = HAL::GetButton(BUTTON::BTN_LEFT);
        auto btn_up = HAL::GetButton(BUTTON::BTN_UP);
        auto btn_down = HAL::GetButton(BUTTON::BTN_DOWN);
        auto btn_mid = HAL::GetButton(BUTTON::BTN_MID);
        auto btn_power = HAL::GetButton(BUTTON::BTN_POWER);

        auto calc_step = [&](uint32_t delta) -> int {
            int step = 10;
            if (delta < 30) step = 30;
            else if (delta < 50) step = 20;
            else if (delta < 100) step = 15;
            else if (delta < 200) step = 10;
            else step = 5;
            return step;
        };

        if (_popup_mode == PopupMode::Brightness) {
            if (btn_right == APP_BUTTON_STATE_CLICKED || btn_left == APP_BUTTON_STATE_CLICKED) {
                uint32_t delta = (_last_scroll_time == 0) ? 1000 : (now - _last_scroll_time);
                _last_scroll_time = now;

                const int step = calc_step(delta);
                if (btn_right == APP_BUTTON_STATE_CLICKED) {
                    if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
                    _brightness_val += step;
                } else {
                    if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
                    _brightness_val -= step;
                }

                if (_brightness_val > 255) _brightness_val = 255;
                if (_brightness_val < 0) _brightness_val = 0;
                HAL::SetDisplayBrightness(static_cast<uint8_t>(_brightness_val));
                if (_view) _view->updateBrightnessPopup((_brightness_val * 100) / 255);
            } else if (btn_mid == APP_BUTTON_STATE_CLICKED) {
                if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(true);
            } else if (btn_power == APP_BUTTON_STATE_CLICKED) {
                if (BtnPower) BtnPower->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(false);
            }
        } else if (_popup_mode == PopupMode::AnimSpeed) {
            if (btn_right == APP_BUTTON_STATE_CLICKED || btn_left == APP_BUTTON_STATE_CLICKED) {
                uint32_t delta = (_last_scroll_time == 0) ? 1000 : (now - _last_scroll_time);
                _last_scroll_time = now;

                int step = calc_step(delta);
                if (step < 5) step = 5;

                if (btn_right == APP_BUTTON_STATE_CLICKED) {
                    if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
                    _ui_anim_speed_pct += step;
                } else {
                    if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
                    _ui_anim_speed_pct -= step;
                }

                if (_ui_anim_speed_pct > 200) _ui_anim_speed_pct = 200;
                if (_ui_anim_speed_pct < 50) _ui_anim_speed_pct = 50;

                char buf[48] = {};
                snprintf(buf, sizeof(buf), AssetPool::GetText().PocketFan_Settings_AnimSpeedFmt, _ui_anim_speed_pct);
                if (_view) _view->updateValuePopup(buf, _ui_anim_speed_pct);
            } else if (btn_mid == APP_BUTTON_STATE_CLICKED) {
                if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(true);
            } else if (btn_power == APP_BUTTON_STATE_CLICKED) {
                if (BtnPower) BtnPower->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(false);
            }
        } else if (_popup_mode == PopupMode::Trackball) {
            bool need_render = false;

            // More intuitive mapping:
            // LEFT/RIGHT -> switch field (Mode/R/G/B/W)
            // UP/DOWN    -> change current field value
            if (btn_left == APP_BUTTON_STATE_CLICKED) {
                if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
                _trackball_field--;
                if (_trackball_field < 0) _trackball_field = 4;
                need_render = true;
            } else if (btn_right == APP_BUTTON_STATE_CLICKED) {
                if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
                _trackball_field++;
                if (_trackball_field > 4) _trackball_field = 0;
                need_render = true;
            }

            if (btn_up == APP_BUTTON_STATE_CLICKED || btn_down == APP_BUTTON_STATE_CLICKED) {
                if (BtnUp && btn_up == APP_BUTTON_STATE_CLICKED) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
                if (BtnDown && btn_down == APP_BUTTON_STATE_CLICKED) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;

                if (_trackball_field == 0) {
                    // Mode field: toggle between Random and Custom.
                    _trackball_mode = (_trackball_mode == 0) ? 1 : 0;
                } else {
                    uint32_t delta = (_last_scroll_time == 0) ? 1000 : (now - _last_scroll_time);
                    _last_scroll_time = now;

                    // Finer color control than generic popup step.
                    int step = 1;
                    if (delta < 40) step = 16;
                    else if (delta < 80) step = 8;
                    else if (delta < 180) step = 4;

                    int* val = nullptr;
                    if (_trackball_field == 1) val = &_trackball_r;
                    else if (_trackball_field == 2) val = &_trackball_g;
                    else if (_trackball_field == 3) val = &_trackball_b;
                    else if (_trackball_field == 4) val = &_trackball_w;

                    if (val) {
                        if (btn_up == APP_BUTTON_STATE_CLICKED) *val += step;
                        else *val -= step;
                        if (*val > 255) *val = 255;
                        if (*val < 0) *val = 0;
                    }
                }
                need_render = true;
            }

            if (need_render) {
                _render_trackball_popup();
            }

            if (btn_mid == APP_BUTTON_STATE_CLICKED) {
                if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(true);
            } else if (btn_power == APP_BUTTON_STATE_CLICKED) {
                if (BtnPower) BtnPower->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(false);
            }
        } else if (_popup_mode == PopupMode::FactoryResetConfirm) {
            if (btn_mid == APP_BUTTON_STATE_CLICKED) {
                if (BtnOk) BtnOk->currentState = APP_BUTTON_STATE_NOCHANGE;
                ESP_LOGW(TAG, "Factory reset requested");
                HAL::FactoryReset(nullptr);
                return;
            }

            if (btn_power == APP_BUTTON_STATE_CLICKED ||
                btn_left == APP_BUTTON_STATE_CLICKED ||
                btn_right == APP_BUTTON_STATE_CLICKED ||
                btn_up == APP_BUTTON_STATE_CLICKED ||
                btn_down == APP_BUTTON_STATE_CLICKED) {
                if (BtnPower) BtnPower->currentState = APP_BUTTON_STATE_NOCHANGE;
                if (BtnLeft) BtnLeft->currentState = APP_BUTTON_STATE_NOCHANGE;
                if (BtnRight) BtnRight->currentState = APP_BUTTON_STATE_NOCHANGE;
                if (BtnUp) BtnUp->currentState = APP_BUTTON_STATE_NOCHANGE;
                if (BtnDown) BtnDown->currentState = APP_BUTTON_STATE_NOCHANGE;
                _exit_popup(false);
            }
        } else if (_popup_mode == PopupMode::EscCalibrating) {
            if (!HAL::IsEscCalibrationRunning()) {
                _exit_popup(true);
            }
        }

        if (_view) {
            _view->update();
        }
        return;
    }

    if (_view) {
        _view->update();
    }

    // Rebuild UI after callbacks (e.g. language switch) to avoid deleting the view
    // while it's still executing its own event handler.
    if (_request_rebuild_view) {
        _request_rebuild_view = false;
        int idx = _rebuild_selected_index;
        _destroy_view();
        _create_view(idx, true);
    }
}

void SettingsApp::onClose()
{
    ESP_LOGI(TAG, "onClose");
    _destroy_view();
}

void SettingsApp::onDestroy()
{
    ESP_LOGI(TAG, "onDestroy");
    _destroy_view();
}

void SettingsApp::_create_view(int initial_index, bool skip_entry_anim)
{
    if (_view) return;
    _view = new SettingsView();

    const auto& tr = AssetPool::GetText();

    // Get initial values
    _popup_mode = PopupMode::None;
    _brightness_val = HAL::GetSystemConfig().brightness;
    _brightness_before = _brightness_val;
    _theme_black = HAL::GetSystemConfig().invertDisplay;
    _ui_anim_speed_pct = HAL::GetSystemConfig().uiAnimSpeedPct;
    if (_ui_anim_speed_pct < 50) _ui_anim_speed_pct = 50;
    if (_ui_anim_speed_pct > 200) _ui_anim_speed_pct = 200;

    int item_index = 0;

    // 0. Screen brightness (0%-100%)
    _view->addSettingsItem({tr.AppSettings_Option_Brightness, false, false, [this]() {
        ESP_LOGI(TAG, "Open brightness popup");
        _enter_popup(PopupMode::Brightness);
    }});
    item_index++;

    // 1. Theme (white/black), default white at boot
    _theme_item_index = item_index;
    _view->addSettingsItem({tr.PocketFan_Settings_Theme, true, _theme_black, [this]() {
        _theme_black = !_theme_black;
        ESP_LOGI(TAG, "Toggle Theme: %s", _theme_black ? "Black" : "White");
        HAL::SetDisplayInvert(_theme_black);
        HAL::GetSystemConfig().invertDisplay = _theme_black;
        HAL::SaveSystemConfig();
        _view->updateItemValue(_theme_item_index, _theme_black);
    }});
    item_index++;

    // 2. ESC calibration
    _view->addSettingsItem({tr.PocketFan_Settings_EscCalibration, false, false, [this]() {
        ESP_LOGI(TAG, "ESC calibration");
        const bool started = HAL::StartEscCalibration();
        if (!started) {
            ESP_LOGW(TAG, "ESC calibration start failed or already running");
            HAL::PopWarning("ESC calibration start failed");
            return;
        }
        _enter_popup(PopupMode::EscCalibrating);
    }});
    item_index++;

    // 3. Trackball color
    _view->addSettingsItem({tr.PocketFan_Settings_Trackball, false, false, [this]() {
        ESP_LOGI(TAG, "Trackball settings");
        _enter_popup(PopupMode::Trackball);
    }});
    item_index++;

    // 4. Animation sensitivity/speed
    _view->addSettingsItem({tr.PocketFan_Settings_AnimSpeed, false, false, [this]() {
        ESP_LOGI(TAG, "Animation speed");
        _enter_popup(PopupMode::AnimSpeed);
    }});
    item_index++;

    // 5. Language
    _language_item_index = item_index;
    {
        char buf[48] = {};
        const LocaleCode_t cur = HAL::GetSystemConfig().localeCode;
        const char* cur_name = (cur == locale_code_cn) ? tr.AppSettings_Option_Chinese : tr.AppSettings_Option_English;
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_LanguageFmt, cur_name);
        _view->addSettingsItem({buf, false, false, [this]() {
            LocaleCode_t cur = HAL::GetSystemConfig().localeCode;
            LocaleCode_t next = (cur == locale_code_cn) ? locale_code_en : locale_code_cn;
            ESP_LOGI(TAG, "Switch Language: %s", next == locale_code_cn ? "CN" : "EN");
            AssetPool::SetLocaleCode(next);
            HAL::GetSystemConfig().localeCode = next;
            HAL::SaveSystemConfig();

            // Rebuild the list with the new locale (done in onRunning).
            _rebuild_selected_index = _view ? _view->getSelectedOptionIndex() : 0;
            _request_rebuild_view = true;
        }});
    }
    item_index++;

    // 6. Factory reset (auto reboot)
    _view->addSettingsItem({tr.PocketFan_Settings_FactoryReset, false, false, [this]() {
        ESP_LOGW(TAG, "Factory reset confirm");
        _enter_popup(PopupMode::FactoryResetConfirm);
    }});
    item_index++;

    // 7. Back
    _view->addSettingsItem({tr.AppSettings_Option_Back, false, false, [this]() {
        ESP_LOGI(TAG, "Back to Menu");
        mooncake::GetMooncake().openApp(APPS::menu_id);
        close();
    }});
    item_index++;

    _view->setInitialIndex(initial_index);
    _view->setSkipEntryAnimation(skip_entry_anim);
    _view->init();
}

void SettingsApp::_destroy_view()
{
    if (_view) {
        delete _view;
        _view = nullptr;
    }
}

void SettingsApp::_enter_popup(PopupMode mode)
{
    if (!_view) return;

    _popup_mode = mode;
    _last_scroll_time = 0;

    const auto& tr = AssetPool::GetText();

    if (mode == PopupMode::Brightness) {
        _brightness_before = _brightness_val;
        _view->showBrightnessPopup((_brightness_val * 100) / 255);
        return;
    }

    if (mode == PopupMode::AnimSpeed) {
        _ui_anim_speed_before = _ui_anim_speed_pct;
        char buf[48] = {};
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_AnimSpeedFmt, _ui_anim_speed_pct);
        _view->showValuePopup(buf, _ui_anim_speed_pct, 50, 200);
        return;
    }

    if (mode == PopupMode::Trackball) {
        const auto& cfg = HAL::GetSystemConfig();
        _trackball_mode = static_cast<int>(cfg.trackballLedMode);
        _trackball_r = cfg.trackballR;
        _trackball_g = cfg.trackballG;
        _trackball_b = cfg.trackballB;
        _trackball_w = cfg.trackballW;

        _trackball_mode_before = _trackball_mode;
        _trackball_r_before = _trackball_r;
        _trackball_g_before = _trackball_g;
        _trackball_b_before = _trackball_b;
        _trackball_w_before = _trackball_w;

        _trackball_field = 0;

        _render_trackball_popup();
        return;
    }

    if (mode == PopupMode::FactoryResetConfirm) {
        _view->showMessagePopup(tr.PocketFan_Settings_FactoryResetConfirm);
        return;
    }

    if (mode == PopupMode::EscCalibrating) {
        _view->showMessagePopup(tr.PocketFan_Settings_EscCalibrating);
        return;
    }
}

void SettingsApp::_exit_popup(bool apply_changes)
{
    if (!_view) {
        _popup_mode = PopupMode::None;
        return;
    }

    const auto mode = _popup_mode;
    _popup_mode = PopupMode::None;

    if (mode == PopupMode::Brightness) {
        if (apply_changes) {
            ESP_LOGI(TAG, "Brightness applied: %d", _brightness_val);
            HAL::GetSystemConfig().brightness = _brightness_val;
            HAL::SaveSystemConfig();
        } else {
            _brightness_val = _brightness_before;
            HAL::SetDisplayBrightness(static_cast<uint8_t>(_brightness_val));
        }
        _view->hidePopup();
        return;
    }

    if (mode == PopupMode::AnimSpeed) {
        if (apply_changes) {
            ESP_LOGI(TAG, "Anim speed applied: %d%%", _ui_anim_speed_pct);
            HAL::GetSystemConfig().uiAnimSpeedPct = _ui_anim_speed_pct;
            HAL::SaveSystemConfig();

            // Rebuild the view to apply new animation durations.
            _rebuild_selected_index = _view ? _view->getSelectedOptionIndex() : 0;
            _request_rebuild_view = true;
        } else {
            _ui_anim_speed_pct = _ui_anim_speed_before;
        }
        _view->hidePopup();
        return;
    }

    if (mode == PopupMode::Trackball) {
        if (apply_changes) {
            ESP_LOGI(TAG, "Trackball applied: mode=%d r=%d g=%d b=%d w=%d",
                     _trackball_mode, _trackball_r, _trackball_g, _trackball_b, _trackball_w);
            auto& cfg = HAL::GetSystemConfig();
            cfg.trackballLedMode = static_cast<CONFIG::TrackballLedMode_t>(_trackball_mode);
            cfg.trackballR = static_cast<uint8_t>(_trackball_r);
            cfg.trackballG = static_cast<uint8_t>(_trackball_g);
            cfg.trackballB = static_cast<uint8_t>(_trackball_b);
            cfg.trackballW = static_cast<uint8_t>(_trackball_w);
            HAL::SaveSystemConfig();
        } else {
            _trackball_mode = _trackball_mode_before;
            _trackball_r = _trackball_r_before;
            _trackball_g = _trackball_g_before;
            _trackball_b = _trackball_b_before;
            _trackball_w = _trackball_w_before;
        }
        apply_trackball_effect_preview(_trackball_mode, _trackball_r, _trackball_g, _trackball_b, _trackball_w);
        _view->hidePopup();
        return;
    }

    if (mode == PopupMode::FactoryResetConfirm || mode == PopupMode::EscCalibrating) {
        _view->hidePopup();
        return;
    }
}

void SettingsApp::_render_trackball_popup()
{
    if (!_view) return;

    const auto& tr = AssetPool::GetText();

    char buf[64] = {};
    int bar_value = 0;

    if (_trackball_field == 0) {
        const char* mode_name =
            (_trackball_mode == CONFIG::TRACKBALL_LED_CUSTOM_STATIC) ? tr.PocketFan_Settings_TrackballMode_Custom
                                                                     : tr.PocketFan_Settings_TrackballMode_Random;
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_TrackballModeFmt, mode_name);
        bar_value = (_trackball_mode == CONFIG::TRACKBALL_LED_CUSTOM_STATIC) ? 255 : 0;
    } else if (_trackball_field == 1) {
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_TrackballRFmt, _trackball_r);
        bar_value = _trackball_r;
    } else if (_trackball_field == 2) {
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_TrackballGFmt, _trackball_g);
        bar_value = _trackball_g;
    } else if (_trackball_field == 3) {
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_TrackballBFmt, _trackball_b);
        bar_value = _trackball_b;
    } else {
        snprintf(buf, sizeof(buf), tr.PocketFan_Settings_TrackballWFmt, _trackball_w);
        bar_value = _trackball_w;
    }

    if (_view->isPopupActive()) {
        _view->updateValuePopup(buf, bar_value);
    } else {
        _view->showValuePopup(buf, bar_value, 0, 255);
    }

    // Live preview while adjusting trackball settings.
    apply_trackball_effect_preview(_trackball_mode, _trackball_r, _trackball_g, _trackball_b, _trackball_w);
}
