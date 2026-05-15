#pragma once
#include "../globals.hpp"

void screenConfigUpdate(Screen &currentScreen, bool &errorConfig, bool &errorCreating, bool &errorUpdating,
                        bool &invalidIp, bool &inputEmpty, bool &fromAdmin);
void screenConfigDraw(Screen &currentScreen, bool &inputEmpty, bool &invalidIp, bool &errorUpdating, bool &errorCreating, bool &errorConfig);
