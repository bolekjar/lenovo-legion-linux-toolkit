// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUIntelPState.h"
#include "../KitLog.h"


namespace LenovoLegionDaemon::Kit {

SysFsDriverCPUIntelPState::SysFsDriverCPUIntelPState() : SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/intel_pstate",{})
{

}

void SysFsDriverCPUIntelPState::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        LOG_D(QString("CPU Intel P-State driver found in path: ") + m_path.c_str());

        if(std::filesystem::exists(std::filesystem::path(m_path).append("hwp_dynamic_boost")))
        {
            m_descriptor["hwp_dynamic_boost"] = std::filesystem::path(m_path).append("hwp_dynamic_boost");
        }

        if(std::filesystem::exists(std::filesystem::path(m_path).append("max_perf_pct")))
        {
            m_descriptor["max_perf_pct"] = std::filesystem::path(m_path).append("max_perf_pct");
        }

        if(std::filesystem::exists(std::filesystem::path(m_path).append("min_perf_pct")))
        {
            m_descriptor["min_perf_pct"] = std::filesystem::path(m_path).append("min_perf_pct");
        }

        if(std::filesystem::exists(std::filesystem::path(m_path).append("no_turbo")))
        {
            m_descriptor["no_turbo"] = std::filesystem::path(m_path).append("no_turbo");
        }

        if(std::filesystem::exists(std::filesystem::path(m_path).append("status")))
        {
            m_descriptor["status"] = std::filesystem::path(m_path).append("status");
        }
    }
    else
    {
        LOG_D(QString("CPU Intel P-State driver not found in path: ") + m_path.c_str());
    }
}


void SysFsDriverCPUIntelPState::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_driver == DRIVER_NAME)
    {
        init();
        validate();

        emit kernelEvent({
            .m_driverName = DRIVER_NAME,
            .m_action = SubsystemEvent::Action::RELOADED,
            .m_DriverSpecificEventType = "reloaded",
            .m_DriverSpecificEventValue = {}
        });
    }
}

}

