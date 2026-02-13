#pragma once

#include <mooncake.h>
#include "../settings/view/settings_view.h"

class EmojiApp : public mooncake::AppAbility {
public:
    EmojiApp() = default;
    ~EmojiApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    std::string _formatTimeoutItemText() const;
    bool _isValidTimeout(int timeout_sec) const;
    int _timeoutOptionIndex(int timeout_sec) const;
    int _adjustTimeout(int timeout_sec, int delta) const;

private:
    SettingsView* _view = nullptr;
};
