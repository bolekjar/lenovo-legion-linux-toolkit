// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once
/*
 * Power limits of the older Legion firmware (SysFsDriverLegionLegacy) in the messages of the newer firmware
 * (CPUPower.Limit, GPUPower.Limit): the firmware has the values of the custom mode only, so the limit has
 * the descriptor of the custom mode only (the GUI shows the limits in the custom mode, the same as God mode of
 * Lenovo Legion Toolkit).
 */
#include "SysFsDataProvider.h"
#include "Drivers/SysFsDriverLegionLegacy.h"

namespace LenovoLegionDaemon::Kit::LegacyLimits {

/*
 * Power mode of the descriptor (PowerProfile POWER_PROFILE_CUSTOM)
 */
inline constexpr int THERMAL_MODE_CUSTOM = 255;

/*
 * Limit read from the firmware, not set when the firmware does not support it
 */
template<class Limit>
void read(const SysFsDriverLegionLegacy::Legacy::Limit& paths,Limit* limit)
{
    if(!paths.exists())
    {
        return;
    }

    auto& descriptor = (*limit->mutable_mode_descriptor_map())[THERMAL_MODE_CUSTOM];

    limit->set_current_value(SysFsDataProvider::getData(paths.m_current_value).toUInt());

    if(!paths.m_default_value.empty())
    {
        descriptor.set_default_value(SysFsDataProvider::getData(paths.m_default_value).toUInt());
    }

    descriptor.set_min_value(SysFsDataProvider::getData(paths.m_min_value).toUInt());
    descriptor.set_max_value(SysFsDataProvider::getData(paths.m_max_value).toUInt());
    descriptor.set_scalar_increment(SysFsDataProvider::getData(paths.m_scalar_increment).toUInt());
    descriptor.set_supported(SysFsDataProvider::getData(paths.m_supported).toUInt());
}

/*
 * Write of a limit of the request (only a limit the firmware supports, the request was checked by settable)
 */
template<class Limit>
void write(const SysFsDriverLegionLegacy::Legacy::Limit& paths,bool has,const Limit& limit)
{
    if(has && paths.exists())
    {
        SysFsDataProvider::setData(paths.m_current_value,static_cast<quint32>(limit.current_value()));
    }
}

}
