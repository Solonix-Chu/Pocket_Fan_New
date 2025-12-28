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
    void _create_view();
    void _destroy_view();
};