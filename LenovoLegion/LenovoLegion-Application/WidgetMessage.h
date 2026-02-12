// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QtTypes>

namespace LenovoLegionGui {

struct WidgetMessage {

    enum class Widget : quint8 {
        POWER_CONTROL        ,
        FAN_CONTROL          ,
        HW_MONITORING        ,
        CPU_CONTROL          ,
        CPU_FREQUENCY_CONTROL,
        POWER_PROFILE_CONTROL,
        BATTERY_STATUS       ,
    };

    enum class Message : quint8 {
        POWER_PROFILE_CHANGED        = 0,
        POWER_PROFILE_CHANGED_CUSTOM = 1,
        POWER_PROFILE_NOT_AVAILABLE  = 2,

        CPU_CONTROL_CHANGED          = 20,
        CPU_CONTROL_NOT_AVAILABLE    = 21,

        CPU_FREQ_CONTROL_APPLY          = 30,
        CPU_FREQ_CONTROL_NOT_AVAILABLE  = 31
    };


    Widget                  m_widget;
    Message                 m_message;
};


}
