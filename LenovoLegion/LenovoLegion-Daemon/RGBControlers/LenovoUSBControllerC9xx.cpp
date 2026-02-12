// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "LenovoUSBControllerC9xx.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

LenovoUSBControllerC9xx::LenovoUSBControllerC9xx(hid_device *dev_handle, const char *path, uint16_t in_pid,uint16_t in_vid) :
    LenovoUSBController(dev_handle,path,in_pid,in_vid)
{}



void LenovoUSBControllerC9xx::setLedsDirectOn(uint8_t profile_id)
{
    LOG_T(QString::asprintf("LenovoUSBController::setLedsDirectOn: Setting direct LED control for profile %d", profile_id));

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::AuroraStartStop, {.value1 = 0x01,
                                                                                             .value2 = profile_id
                                                                                         }));
}

void LenovoUSBControllerC9xx::setLedsDirectOff(uint8_t profile_id)
{
    LOG_T(QString::asprintf("LenovoUSBController::setLedsDirectOff: Disabling direct LED control for profile %d", profile_id));

    LenovoUSBController::sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::AuroraStartStop, {.value1 = 0x02,
                                                                                             .value2 = profile_id
                                                                                         }));
}

void LenovoUSBControllerC9xx::setLedsDirect(const std::vector<RGBColor> &colors)
{
    LOG_T("LenovoUSBController::setLedsDirect: Setting direct LED colors");

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::AuroraSendBitmap,{},[this,&colors](){
        ByteArray payload;

        for(size_t height = 0; height < getKeyMap().m_height; height++)
        {
            for (int width = 0; width < getKeyMap().m_width; ++width) {

                if(getKeyMap().m_keyCodes[width][height])
                    continue;

                payload.push_back(getKeyMap().m_keyCodes[width][height] & 0xFF);
                payload.push_back(getKeyMap().m_keyCodes[width][height] >> 8 & 0xFF);
                payload.push_back(RGBGetRValue(colors[width + (height * getKeyMap().m_width)]));
                payload.push_back(RGBGetGValue(colors[width + (height * getKeyMap().m_width)]));
                payload.push_back(RGBGetBValue(colors[width + (height * getKeyMap().m_width)]));

            }
        }
        return payload;
    }()));
}

}
