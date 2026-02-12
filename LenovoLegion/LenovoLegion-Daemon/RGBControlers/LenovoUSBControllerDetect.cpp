// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "LenovoRGBControllerC9xx.h"
#include "LenovoRGBControllerC197.h"

#include "RGBControllerDetector.h"

namespace LenovoLegionDaemon {

/*-----------------------------------------------------*\
| vendor ID                                            |
\*-----------------------------------------------------*/
#define ITE_VID                                 0x048D


RGBController * DetectLenovoLegionUSBControllersC9xx(const hid_device_info& info, const std::string& name)
{
    hid_device* dev = hid_open_path(info.path);

    if(dev)
    {
        LenovoRGBController*     rgb_controller  = new LenovoRGBControllerC9xx(new LenovoUSBControllerC9xx(dev, info.path, info.product_id,info.vendor_id));
        rgb_controller->GetName()                     = name;

        return rgb_controller;
    }

    return nullptr;
}

/*
 * Todo : Verify if this works on actual C197 devices
 */
RGBController * DetectLenovoLegionUSBControllersC197(const hid_device_info& info, const std::string& name)
{
    hid_device* dev = hid_open_path(info.path);

    if(dev)
    {
        LenovoRGBController*     rgb_controller  = new LenovoRGBControllerC197(new LenovoUSBController(dev, info.path, info.product_id,info.vendor_id));
        rgb_controller->GetName()                     = name;

        return rgb_controller;
    }

    return nullptr;
}

REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC9xx, ITE_VID, 0xC900, 0xFF00);
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC197, ITE_VID, 0xC197, 0xFFFF);
}
