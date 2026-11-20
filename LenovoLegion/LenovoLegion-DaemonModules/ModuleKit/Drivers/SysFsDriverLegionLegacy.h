// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include "../SysFsDriver.h"
#include "SysFsDriverLegion.h"

namespace LenovoLegionDaemon::Kit {

/*
 * CPU and GPU power limits of the older Legion firmware (lenovo_legion legion-wmi-cpu-gpu.c, SmartFan 4 / 5 or
 * LegionZone 1 / 2): firmware attributes legion-wmi-cpu-0 / legion-wmi-gpu-0 instead of the other mode. A limit is
 * present only when the firmware supports it, its values are single values of the custom mode (not per mode).
 */
class SysFsDriverLegionLegacy : public SysFsDriver
{
public:

    struct Legacy {

        struct Limit {

            Limit(const SysFsDriver::DescriptorType& descriptor,const QString& name) :
                m_current_value(descriptor.value(name + "_current_value")),
                m_default_value(descriptor.value(name + "_default_value")),
                m_min_value(descriptor.value(name + "_min_value")),
                m_max_value(descriptor.value(name + "_max_value")),
                m_scalar_increment(descriptor.value(name + "_scalar_increment")),
                m_supported(descriptor.value(name + "_supported"))
            {}

            bool exists() const { return !m_current_value.empty() && !m_supported.empty(); }

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_default_value;        // empty: the firmware has no default value
            const std::filesystem::path m_min_value;
            const std::filesystem::path m_max_value;
            const std::filesystem::path m_scalar_increment;
            const std::filesystem::path m_supported;
        };

        struct CPU {
            explicit CPU(const SysFsDriver::DescriptorType& descriptor) :
                m_cpu_ltp_limit(descriptor,"cpu_ltp_limit"),
                m_cpu_stp_limit(descriptor,"cpu_stp_limit"),
                m_cpu_pp_limit(descriptor,"cpu_pp_limit"),
                m_cpu_clp_limit(descriptor,"cpu_clp_limit"),
                m_apus_pptp_limit(descriptor,"apus_pptp_limit"),
                m_cpu_tmp_limit(descriptor,"cpu_tmp_limit")
            {}

            const Limit m_cpu_ltp_limit;
            const Limit m_cpu_stp_limit;
            const Limit m_cpu_pp_limit;
            const Limit m_cpu_clp_limit;
            const Limit m_apus_pptp_limit;
            const Limit m_cpu_tmp_limit;
        };

        struct GPU {
            explicit GPU(const SysFsDriver::DescriptorType& descriptor) :
                m_gpu_power_boost(descriptor,"gpu_power_boost"),
                m_gpu_configurable_tgp(descriptor,"gpu_configurable_tgp"),
                m_gpu_temperature_limit(descriptor,"gpu_temperature_limit")
            {}

            const Limit m_gpu_power_boost;
            const Limit m_gpu_configurable_tgp;
            const Limit m_gpu_temperature_limit;
        };
    };

    /*
     * EVENT_TYPE of the uevents of the class devices (legion-common.h)
     */
    enum EventType : int {
        LEGACY_CPU_LTP      = 4000,
        LEGACY_CPU_STP      = 4001,
        LEGACY_CPU_PP       = 4002,
        LEGACY_CPU_CLP      = 4003,
        LEGACY_CPU_TMP      = 4004,
        LEGACY_APU_SPPT     = 4005,
        LEGACY_GPU_PB       = 4010,
        LEGACY_GPU_TGP      = 4011,
        LEGACY_GPU_TMP      = 4012
    };

public:

    ~SysFsDriverLegionLegacy() override = default;

    virtual void init() override;

    void handleKernelEvent(const KernelEvent::Event &event) override;

public:

    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;

    /*
     * Class device of the limits (uevents add, remove, change)
     */
    static constexpr const char* SUBSYSTEM   =  "legion-firmware-attributes";

protected:

    SysFsDriverLegionLegacy(const char* driverName,const char* sysName);

private:

    const QString m_sysName;
};

class SysFsDriverLegionLegacyCPU : public SysFsDriverLegionLegacy
{
public:
    SysFsDriverLegionLegacyCPU() : SysFsDriverLegionLegacy(DRIVER_NAME,SYS_NAME) {}

    static constexpr const char* DRIVER_NAME =  "legion_wmi_cpu";
    static constexpr const char* SYS_NAME    =  "legion-wmi-cpu-0";
};

class SysFsDriverLegionLegacyGPU : public SysFsDriverLegionLegacy
{
public:
    SysFsDriverLegionLegacyGPU() : SysFsDriverLegionLegacy(DRIVER_NAME,SYS_NAME) {}

    static constexpr const char* DRIVER_NAME =  "legion_wmi_gpu";
    static constexpr const char* SYS_NAME    =  "legion-wmi-gpu-0";
};

}
