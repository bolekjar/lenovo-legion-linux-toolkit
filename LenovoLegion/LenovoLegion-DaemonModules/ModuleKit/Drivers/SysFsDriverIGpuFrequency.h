// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../SysFsDriver.h"

namespace LenovoLegionDaemon::Kit {

/*
 * Frequency of the Intel integrated GPU: i915 (gt_min / max / boost / RPn / RP0 / RP1 / act_freq_mhz of the DRM card) or
 * xe (device/tile0/gt0/freq0: min / max / rpn / rp0 / rpe / act_freq). The first DRM card of these drivers.
 */
class SysFsDriverIGpuFrequency : public SysFsDriver
{
public:

    struct Frequency {

        Frequency(const SysFsDriver::DescriptorType& descriptor) :
            m_driver(descriptor.contains("i915") ? "i915" : "xe"),
            m_min(descriptor.value("min")),
            m_max(descriptor.value("max")),
            m_boost(descriptor.value("boost")),
            m_rpn(descriptor.value("rpn")),
            m_rp0(descriptor.value("rp0")),
            m_efficient(descriptor.value("efficient")),
            m_current(descriptor.value("current"))
        {}

        const QString               m_driver;
        const std::filesystem::path m_min;         // rw
        const std::filesystem::path m_max;         // rw
        const std::filesystem::path m_boost;       // rw, i915 only (empty path otherwise)
        const std::filesystem::path m_rpn;         // lowest frequency of the hardware
        const std::filesystem::path m_rp0;         // highest frequency of the hardware
        const std::filesystem::path m_efficient;   // RP1 / RPe
        const std::filesystem::path m_current;     // actual frequency
    };

public:

    explicit SysFsDriverIGpuFrequency();
    ~SysFsDriverIGpuFrequency() override = default;

    void init() override;

public:

    static constexpr const char* DRIVER_NAME =  "__system_igpu_frequency__";
};

}
