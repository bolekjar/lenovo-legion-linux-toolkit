// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "DataProviderRGBController.h"

namespace LenovoLegionDaemon {

class HIDDeviceDetector
{

public:

    HIDDeviceDetector(std::string name,
                      DataProviderRGBController::HIDDeviceDetectorFunction detector,
                      uint16_t vid,
                      uint16_t pid,
                      uint16_t pidMask)
    {
        DataProviderRGBController::registerControler(name,detector,vid,pid,pidMask);
    }
};


#define REGISTER_HID_DETECTOR(name, func, vid, pid,pid_mask) \
    static HIDDeviceDetector device_detector_obj_##vid##pid##__##page##_##usage(name,       \
                                                                                func,       \
                                                                                vid,        \
                                                                                pid,        \
                                                                                pid_mask)
}
