// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * HID report descriptor of a device (hidapi), used by the modules of the RGB keyboards (header only: ModuleKit does not
 * link hidapi, only the modules which include it)
 */
#include <hidapi.h>

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace LenovoLegionDaemon::Kit {

/*
 * Length of a feature report in bytes (with the report ID) from the HID report descriptor: 0 when the descriptor has
 * no such report, empty when the descriptor can not be read (hidapi older than 0.14).
 * Only the items needed for it: Report Size, Report Count, Report ID, Push / Pop (global) and Feature (main).
 */
inline std::optional<size_t> hidFeatureReportLength(hid_device* dev,uint8_t reportId)
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

}
