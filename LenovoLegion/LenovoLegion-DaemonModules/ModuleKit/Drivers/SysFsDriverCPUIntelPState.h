// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../SysFsDriver.h"


namespace LenovoLegionDaemon::Kit {

class SysFsDriverCPUIntelPState : public SysFsDriver
{
public:

    struct IntelPState {

        IntelPState(const SysFsDriver::DescriptorType& descriptor) :
            m_hwp_dynamic_boost(descriptor.find("hwp_dynamic_boost") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["hwp_dynamic_boost"]),
            m_max_perf_pct(descriptor.find("max_perf_pct") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["max_perf_pct"]),
            m_min_perf_pct(descriptor.find("min_perf_pct") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["min_perf_pct"]),
            m_no_turbo(descriptor.find("no_turbo") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["no_turbo"]),
            m_status(descriptor.find("status") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["status"])
        {}

        const std::optional<std::filesystem::path> m_hwp_dynamic_boost;
        const std::optional<std::filesystem::path> m_max_perf_pct;
        const std::optional<std::filesystem::path> m_min_perf_pct;
        const std::optional<std::filesystem::path> m_no_turbo;
        const std::optional<std::filesystem::path> m_status;
     };

public:

    explicit SysFsDriverCPUIntelPState();
    ~SysFsDriverCPUIntelPState() override = default;


    void init() override;

    void handleKernelEvent(const KernelEvent::Event &event) override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "__system_intel_pstate__";
};

}

