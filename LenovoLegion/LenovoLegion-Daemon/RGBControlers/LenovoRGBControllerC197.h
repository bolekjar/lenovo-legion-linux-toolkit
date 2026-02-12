// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "LenovoRGBController.h"
#include "LenovoUSBController.h"


namespace LenovoLegionDaemon {

class LenovoRGBControllerC197 : public LenovoRGBController
{

public:

    LenovoRGBControllerC197(LenovoUSBController* controller_ptr);
};

}
