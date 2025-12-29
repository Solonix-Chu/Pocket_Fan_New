#pragma once
#include <mooncake.h>

class HomepageApp : public mooncake::AppAbility
{
public:
    HomepageApp() = default;
    ~HomepageApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    void _create_view();
    void _destroy_view();
};
