// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once


#include "LenovoUSBController.h"

namespace LenovoLegionDaemon {

class LenovoUSBControllerC9xx : public LenovoUSBController
{
public:


    LenovoUSBControllerC9xx(hid_device* dev_handle, const char* path, uint16_t in_pid,uint16_t in_vid);


    /*
     * Direct control support
     */
    void setLedsDirect(const std::vector<RGBColor> &colors);

    void setLedsDirectOn(uint8_t profile_id);

    void setLedsDirectOff(uint8_t profile_id);

};

}
