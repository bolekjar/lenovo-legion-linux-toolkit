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
 * Length of a feature report in bytes (with the report ID) from the HID report descriptor: 0 when the descriptor has
 * no such report, empty when the descriptor can not be read (hidapi older than 0.14).
 * Only the items needed for it: Report Size, Report Count, Report ID, Push / Pop (global) and Feature (main).
 */
static std::optional<size_t> featureReportLength(hid_device* dev,uint8_t reportId)
{
#if HID_API_VERSION >= HID_API_MAKE_VERSION(0, 14, 0)
    std::vector<unsigned char> descriptor(HID_API_MAX_REPORT_DESCRIPTOR_SIZE);

    const int length = hid_get_report_descriptor(dev,descriptor.data(),descriptor.size());

    if(length <= 0)
    {
        return std::nullopt;
    }

    struct Globals
    {
        uint32_t size  = 0;
        uint32_t count = 0;
        uint8_t  id    = 0;
    };

    Globals                     globals;
    std::vector<Globals>        stack;
    std::map<uint8_t,uint64_t>  featureBits;

    for(int i = 0; i < length;)
    {
        const uint8_t prefix = descriptor[i];

        /* Long item: bDataSize in the next byte */
        if(prefix == 0xFE)
        {
            if(i + 1 >= length)
            {
                break;
            }

            i += 3 + descriptor[i + 1];
            continue;
        }

        const int     size = (prefix & 0x03) == 3 ? 4 : (prefix & 0x03);
        const uint8_t type = (prefix >> 2) & 0x03;
        const uint8_t tag  = (prefix >> 4) & 0x0F;

        if(i + 1 + size > length)
        {
            break;
        }

        uint32_t data = 0;

        for(int b = 0; b < size; ++b)
        {
            data |= static_cast<uint32_t>(descriptor[i + 1 + b]) << (8 * b);
        }

        if(type == 1)
        {
            switch(tag)
            {
            case 0x7: globals.size  = data;                          break;
            case 0x8: globals.id    = static_cast<uint8_t>(data);    break;
            case 0x9: globals.count = data;                          break;
            case 0xA: stack.push_back(globals);                      break;
            case 0xB: if(!stack.empty()) { globals = stack.back(); stack.pop_back(); } break;
            default:                                                 break;
            }
        }
        else if(type == 0 && tag == 0xB)
        {
            featureBits[globals.id] += static_cast<uint64_t>(globals.size) * globals.count;
        }

        i += 1 + size;
    }

    const auto it = featureBits.find(reportId);

    if(it == featureBits.end())
    {
        return 0;
    }

    return static_cast<size_t>((it->second + 7) / 8) + (reportId != 0 ? 1 : 0);
#else
    (void)dev;
    (void)reportId;
    return std::nullopt;
#endif
}

/*
 * The 4-zone keyboards (e.g. Legion 5 15IAH7H, 048d:c975) have the same IDs as the per-key keyboards (048d:c9xx),
 * they differ in the length of the feature report (4-zone 33 bytes, per-key 960 bytes, the same test as Lenovo Legion
 * Toolkit). A 4-zone keyboard is not opened as a per-key one: no request of the per-key protocol is sent to it.
 */
static bool isPerKeyKeyboard(hid_device* dev,const hid_device_info& info)
{
    const auto length = featureReportLength(dev,REPORT_ID);

    if(!length.has_value())
    {
        LOG_W(QString::asprintf("RGB keyboard %04x:%04x: report descriptor not available (hidapi < 0.14), used as a per-key keyboard",info.vendor_id,info.product_id));
        return true;
    }

    if(*length != PACKET_SIZE)
    {
        LOG_W(QString::asprintf("RGB keyboard %04x:%04x is not a per-key keyboard (feature report 0x%02x: %zu bytes, per-key %d bytes; e.g. a 4-zone keyboard), not supported",
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
 * and LOQ. Every candidate must have the per-key feature report (960 bytes) and answer the compatibility request,
 * other devices of the ranges (4-zone keyboards, e.g. the light bar 048d:c193) are skipped.
 */
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC9xx, ITE_VID, 0xC900, 0xFF00);
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC197, ITE_VID, 0xC100, 0xFF00);
REGISTER_HID_DETECTOR("Lenovo Laptop",   DetectLenovoLegionUSBControllersC197, ITE_VID, 0xC600, 0xFF00);
}
