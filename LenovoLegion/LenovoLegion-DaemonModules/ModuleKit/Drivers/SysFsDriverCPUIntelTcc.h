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
 * Intel TCC offset (processor thermal device, int340x proc_thermal: tcc_offset_degree_celsius of the PCI device): the
 * CPU throttles at TjMax - offset. TjMax from coretemp (temp1_crit of the package). Loaded only with the attribute.
 */
class SysFsDriverCPUIntelTcc : public SysFsDriver
{
public:

    struct Tcc {

        Tcc(const SysFsDriver::DescriptorType& descriptor) :
            m_offset(descriptor.value("tcc_offset")),
            m_tjMax(descriptor.value("tjmax"))
        {}

        const std::filesystem::path m_offset;   // tcc_offset_degree_celsius (rw)
        const std::filesystem::path m_tjMax;    // coretemp temp1_crit (m°C), empty path without it
    };

public:

    explicit SysFsDriverCPUIntelTcc();
    ~SysFsDriverCPUIntelTcc() override = default;

    void init() override;

public:

    static constexpr const char* DRIVER_NAME =  "__system_intel_tcc_offset__";
};

}
