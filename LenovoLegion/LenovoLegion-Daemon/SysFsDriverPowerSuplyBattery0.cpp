// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverPowerSuplyBattery0.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverPowerSuplyBattery0::SysFsDriverPowerSuplyBattery0(QObject* parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/power_supply/BAT0/",{"power_supply",{}},parrent) {}

void SysFsDriverPowerSuplyBattery0::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        LOG_D(QString("Found Power supply battery driver in path: ") + m_path.c_str());

        m_descriptor["powerSuplyBattery0"] = std::filesystem::path(m_path).append("status");
    }
    else
    {
        LOG_T(QString("Power supply battery driver not found in path: ") + m_path.c_str());
    }
}

void SysFsDriverPowerSuplyBattery0::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_sysName == "BAT0")
    {
        if(event.m_action == "change")
        {
            emit kernelEvent({
                .m_driverName = DRIVER_NAME,
                .m_action = SubsystemEvent::Action::CHANGED,
                .m_DriverSpecificEventType = "changed",
                .m_DriverSpecificEventValue = {}
            });
        }
        else
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


}
