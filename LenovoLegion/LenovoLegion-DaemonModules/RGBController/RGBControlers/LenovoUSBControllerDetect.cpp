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

#include <HidReportDescriptor.h>
#include <KitLog.h>

#include <map>
#include <optional>
#include <vector>

namespace LenovoLegionDaemon::Kit {

/*-----------------------------------------------------*\
| vendor ID                                            |
\*-----------------------------------------------------*/
#define ITE_VID                                 0x048D


/*
 * The 4-zone keyboards (e.g. Legion 5 15IAH7H, 048d:c975) have the same IDs as the per-key keyboards (048d:c9xx),
 * they differ in the length of the feature report (4-zone 33 bytes, per-key 960 bytes, the same test as Lenovo Legion
 * Toolkit). A 4-zone keyboard is not opened as a per-key one: no request of the per-key protocol is sent to it.
 */
static bool isPerKeyKeyboard(hid_device* dev,const hid_device_info& info)
{
    const auto length = hidFeatureReportLength(dev,REPORT_ID);

    if(!length.has_value())
    {
        LOG_W(QString::asprintf("RGB keyboard %04x:%04x: report descriptor not available (hidapi < 0.14), used as a per-key keyboard",info.vendor_id,info.product_id));
        return true;
    }

    if(*length != PACKET_SIZE)
    {
        /*
         * A 4-zone keyboard (feature report 0xCC of 33 bytes on the same device) is expected here, the module RGB4Zone
         * handles it: only a debug message
         */
        const auto fourZone = hidFeatureReportLength(dev,0xCC);

        if(fourZone.has_value() && *fourZone == 33)
        {
            LOG_D(QString::asprintf("RGB keyboard %04x:%04x is a 4-zone keyboard (feature report 0x%02x: %zu bytes), handled by the module RGB4Zone",
                                    info.vendor_id,info.product_id,REPORT_ID,*length));
            return false;
        }

        LOG_W(QString::asprintf("RGB keyboard %04x:%04x is not a per-key keyboard (feature report 0x%02x: %zu bytes, per-key %d bytes), skipped",
                                info.vendor_id,info.product_id,REPORT_ID,*length,PACKET_SIZE));
        return false;
    }

    return true;
}

RGBController * DetectLenovoLegionUSBControllersC9xx(const hid_device_info& info, const std::string& name)
{
    hid_device* dev = hid_open_path(info.path);

    if(dev && !isPerKeyKeyboard(dev,info))
    {
        hid_close(dev);
        return nullptr;
    }

    if(dev)
    {
        LenovoRGBController*     rgb_controller  = new LenovoRGBControllerC9xx(new LenovoUSBControllerC9xx(dev, info.path, info.product_id,info.vendor_id));
        rgb_controller->GetName()                     = name;

        return rgb_controller;
    }

    return nullptr;
}

/*
 * Gen 10 and newer (protocol of 048d:c197, Legion Pro 7 16IAX10H): the key map is read from the device
 */
RGBController * DetectLenovoLegionUSBControllersC197(const hid_device_info& info, const std::string& name)
{
    hid_device* dev = hid_open_path(info.path);

    if(dev && !isPerKeyKeyboard(dev,info))
    {
        hid_close(dev);
        return nullptr;
    }

    if(dev)
    {
        LenovoRGBController*     rgb_controller  = new LenovoRGBControllerC197(new LenovoUSBController(dev, info.path, info.product_id,info.vendor_id));
        rgb_controller->GetName()                     = name;

        return rgb_controller;
    }

    return nullptr;
}

/*
 * Per-key keyboards by the ranges of Lenovo Legion Toolkit (Devices.GetKeyboardConfig): 048d:c9xx Legion 9 and older
 * generations (Gen 7 - 9), 048d:c1xx Legion 5 / Pro 5 / 7 / Pro 7 Gen 10 (c197, c195), 048d:c6xx Legion 5 / 7 Gen 11
 * and LOQ. Every candidate must be the HID collection 0xFF89:0x07, have the per-key feature report (960 bytes) and answer
 * the compatibility request; other devices of the ranges (4-zone keyboards, 048d:c193 next to 048d:c197) are skipped.
 */
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC9xx, ITE_VID, 0xC900, 0xFF00);
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC197, ITE_VID, 0xC100, 0xFF00);
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC197, ITE_VID, 0xC600, 0xFF00);
}
