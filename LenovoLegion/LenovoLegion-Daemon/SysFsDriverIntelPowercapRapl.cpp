// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverIntelPowercapRapl.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverIntelPowercapRapl::SysFsDriverIntelPowercapRapl(QObject* parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/powercap/",{"powercap",{}},parrent) {}

void SysFsDriverIntelPowercapRapl::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/")))
    {
        LOG_D(QString("Found Intel Powercap RAPL driver in path: ") + m_path.c_str());

        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_max_power_uw")))
            m_descriptor["ltp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_time_window_us")))
            m_descriptor["ltp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_name")))
            m_descriptor["ltp_name"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_power_limit_uw")))
            m_descriptor["ltp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_0_power_limit_uw");


        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_max_power_uw")))
            m_descriptor["stp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_time_window_us")))
            m_descriptor["stp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_name")))
            m_descriptor["stp_name"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_power_limit_uw")))
            m_descriptor["stp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_1_power_limit_uw");


        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_max_power_uw")))
            m_descriptor["pp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_time_window_us")))
            m_descriptor["pp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_name")))
            m_descriptor["pp_name"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_power_limit_uw")))
            m_descriptor["pp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl:0/constraint_2_power_limit_uw");

        m_descriptor["max_energy_range"] = std::filesystem::path(m_path).append("intel-rapl:0/max_energy_range_uj");
        m_descriptor["powercapCPUEnergy"] = std::filesystem::path(m_path).append("intel-rapl:0/energy_uj");

        m_descriptor["enabled"] = std::filesystem::path(m_path).append("intel-rapl:0/enabled");
    }
    else
    {
        LOG_T(QString("Intel Powercap RAPL driver not found in path: ") + m_path.c_str());
    }

    if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/")))
    {
        LOG_D(QString("Found Intel Powercap RAPL driver in path: ") + m_path.c_str());

        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_max_power_uw")))
            m_descriptor["mmio_ltp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_time_window_us")))
            m_descriptor["mmio_ltp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_name")))
            m_descriptor["mmio_ltp_name"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_power_limit_uw")))
            m_descriptor["mmio_ltp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_0_power_limit_uw");


        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_max_power_uw")))
            m_descriptor["mmio_stp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_time_window_us")))
            m_descriptor["mmio_stp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_name")))
            m_descriptor["mmio_stp_name"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_power_limit_uw")))
            m_descriptor["mmio_stp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_1_power_limit_uw");


        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_max_power_uw")))
            m_descriptor["mmio_pp_max_power_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_max_power_uw");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_time_window_us")))
            m_descriptor["mmio_pp_time_window_us"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_time_window_us");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_name")))
            m_descriptor["mmio_pp_name"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_name");
        if(std::filesystem::exists(std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_power_limit_uw")))
            m_descriptor["mmio_pp_power_limit_uw"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/constraint_2_power_limit_uw");

        m_descriptor["mmio_max_energy_range"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/max_energy_range_uj");
        m_descriptor["mmio_powercapCPUEnergy"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/energy_uj");

        m_descriptor["mmio_enabled"] = std::filesystem::path(m_path).append("intel-rapl-mmio:0/enabled");
    }
    else
    {
        LOG_T(QString("Intel Powercap RAPL driver not found in path: ") + m_path.c_str());
    }
}

void SysFsDriverIntelPowercapRapl::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_sysName == "intel-rapl:0")
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
