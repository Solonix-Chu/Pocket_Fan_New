#pragma once
#include <mooncake.h>
#include "view/menu_view.h"

class MenuApp : public mooncake::AppAbility
{
public:
    MenuApp() = default;
    ~MenuApp() override = default;

    void onCreate() override;
    void onOpen() override;
    void onRunning() override;
    void onClose() override;
    void onDestroy() override;

private:
    MenuView* _view = nullptr;
    uint32_t _last_input_time = 0;
    void _create_view();
    void _destroy_view();
};