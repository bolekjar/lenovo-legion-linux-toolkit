// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once
/*
 * Power limits of the older Legion firmware (SysFsDriverLegionCpuGpu) in the messages of the newer firmware
 * (CPUPower.Limit, GPUPower.Limit): the firmware has the values of the custom mode only, so the limit has
 * the descriptor of the custom mode only (the GUI shows the limits in the custom mode, the same as God mode of
 * Lenovo Legion Toolkit).
 */
#include "SysFsDataProvider.h"
#include "Drivers/SysFsDriverLegionCpuGpu.h"

namespace LenovoLegionDaemon::Kit::CpuGpuLimits {

/*
 * Power mode of the descriptor (PowerProfile POWER_PROFILE_CUSTOM)
 */
inline constexpr int THERMAL_MODE_CUSTOM = 255;

/*
 * Limit read from the firmware, not set when the firmware does not support it
 */
template<class Limit>
void read(const SysFsDriverLegionCpuGpu::CpuGpu::Limit& paths,Limit* limit)
{
    if(!paths.exists())
    {
        return;
    }

    auto& descriptor = (*limit->mutable_mode_descriptor_map())[THERMAL_MODE_CUSTOM];

    limit->set_current_value(SysFsDataProvider::getData(paths.m_current_value).toUInt());

    descriptor.set_min_value(SysFsDataProvider::getData(paths.m_min_value).toUInt());
    descriptor.set_max_value(SysFsDataProvider::getData(paths.m_max_value).toUInt());
    descriptor.set_scalar_increment(SysFsDataProvider::getData(paths.m_scalar_increment).toUInt());
    descriptor.set_supported(SysFsDataProvider::getData(paths.m_supported).toUInt());

    /*
     * Default values "<mode>=<value>,..." (power modes of the firmware and the custom mode, the same as the other mode),
     * a single value (older kernel module) is the default of the custom mode
     */
    if(!paths.m_default_value.empty())
    {
        const QString defaults = SysFsDataProvider::getData(paths.m_default_value);

        if(!defaults.contains('='))
        {
            descriptor.set_default_value(defaults.toUInt());
        }

        for(const QString& pair : defaults.split(',',Qt::SkipEmptyParts))
        {
            const auto modeValue = pair.split('=');
            bool       modeOk    = false;
            bool       valueOk   = false;

            if(modeValue.size() != 2)
            {
                continue;
            }

            const int  mode  = modeValue.at(0).trimmed().toInt(&modeOk);
            const uint value = modeValue.at(1).trimmed().toUInt(&valueOk);

            if(modeOk && valueOk)
            {
                (*limit->mutable_mode_descriptor_map())[mode].set_default_value(value);
            }
        }
    }
}

/*
 * Write of a limit of the request (only a limit the firmware supports, the request was checked by settable)
 */
template<class Limit>
void write(const SysFsDriverLegionCpuGpu::CpuGpu::Limit& paths,bool has,const Limit& limit)
{
    if(has && paths.exists())
    {
        SysFsDataProvider::setData(paths.m_current_value,static_cast<quint32>(limit.current_value()));
    }
}

}
