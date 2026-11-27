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
 * Intel uncore frequency (intel_uncore_frequency, intel_uncore_frequency_tpmi): the frequency of the ring, the last level
 * cache and the memory controller, one directory per package / die (package_XX_die_YY) or per domain of the newer CPUs
 * (uncoreNN). Loaded only with the driver (Intel CPUs).
 */
class SysFsDriverCPUIntelUncore : public SysFsDriver
{
public:

    struct Domain {
        std::filesystem::path m_minFreq;          // min_freq_khz (rw)
        std::filesystem::path m_maxFreq;          // max_freq_khz (rw)
        std::filesystem::path m_initialMinFreq;   // initial_min_freq_khz: the lowest limit of the hardware
        std::filesystem::path m_initialMaxFreq;   // initial_max_freq_khz: the highest limit of the hardware
        std::filesystem::path m_currentFreq;      // current_freq_khz, empty path without it
    };

    struct Uncore {

        Uncore(const SysFsDriver::DescriptorType& descriptor)
        {
            for(auto it = descriptor.cbegin(); it != descriptor.cend(); ++it)
            {
                if(!it.key().endsWith("/min_freq_khz") || it.key().endsWith("initial_min_freq_khz"))
                {
                    continue;
                }

                const QString domain = it.key().left(it.key().lastIndexOf('/') + 1);

                m_domains.push_back(Domain {
                    .m_minFreq        = descriptor.value(domain + "min_freq_khz"),
                    .m_maxFreq        = descriptor.value(domain + "max_freq_khz"),
                    .m_initialMinFreq = descriptor.value(domain + "initial_min_freq_khz"),
                    .m_initialMaxFreq = descriptor.value(domain + "initial_max_freq_khz"),
                    .m_currentFreq    = descriptor.value(domain + "current_freq_khz")
                });
            }
        }

        std::vector<Domain> m_domains;
    };

public:

    explicit SysFsDriverCPUIntelUncore();
    ~SysFsDriverCPUIntelUncore() override = default;

    void init() override;

public:

    static constexpr const char* DRIVER_NAME =  "__system_intel_uncore_frequency__";
};

}
