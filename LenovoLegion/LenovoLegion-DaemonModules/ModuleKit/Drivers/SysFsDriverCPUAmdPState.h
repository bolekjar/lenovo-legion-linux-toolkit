// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../SysFsDriver.h"

#include <vector>

namespace LenovoLegionDaemon::Kit {

/*
 * AMD P-State driver (amd-pstate): /sys/devices/system/cpu/amd_pstate (status, prefcore) and the files of the cpufreq
 * policies it handles: boost (global cpufreq/boost, or of every policy) and the energy performance preference (EPP,
 * only in the active mode). Loaded only with amd-pstate (amd_pstate/status exists).
 */
class SysFsDriverCPUAmdPState : public SysFsDriver
{
public:

    struct AmdPState {

        AmdPState(const SysFsDriver::DescriptorType& descriptor) :
            m_status(descriptor.value("status")),
            m_prefcore(descriptor.value("prefcore")),
            m_eppAvailable(descriptor.value("epp_available"))
        {
            for(auto it = descriptor.cbegin(); it != descriptor.cend(); ++it)
            {
                if(it.key().startsWith("boost"))
                {
                    m_boost.push_back(it.value());
                }
                else if(it.key().startsWith("epp/"))
                {
                    m_epp.push_back(it.value());
                }
            }
        }

        const std::filesystem::path        m_status;         // active, passive, guided, disable
        const std::filesystem::path        m_prefcore;       // enabled, disabled (read only), empty path without it
        const std::filesystem::path        m_eppAvailable;   // EPP values of the first policy, empty path without EPP
        std::vector<std::filesystem::path> m_boost;          // global cpufreq/boost, or boost of every policy
        std::vector<std::filesystem::path> m_epp;            // energy_performance_preference of every policy
    };

public:

    explicit SysFsDriverCPUAmdPState();
    ~SysFsDriverCPUAmdPState() override = default;

    void init() override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "__system_amd_pstate__";
};

}
