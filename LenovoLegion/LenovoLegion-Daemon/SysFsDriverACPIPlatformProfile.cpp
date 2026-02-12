// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverACPIPlatformProfile.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverACPIPlatformProfile::SysFsDriverACPIPlatformProfile(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/firmware/acpi/",{"platform-profile",{}},parrent) {}

void SysFsDriverACPIPlatformProfile::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        if(std::filesystem::exists(std::filesystem::path(m_path).append("platform_profile")))
        {
            LOG_D(QString("Found ACPI platform profile driver in path: ") + m_path.c_str());

            m_descriptor["acpiPlatformProfile"] = std::filesystem::path(m_path).append("platform_profile");
        }
        else
        {
            LOG_T(QString("ACPI Platform Profile driver not found in path: ") + m_path.c_str());
        }
    }
    else
    {
        LOG_T(QString("ACPI Platform Profile driver not found in path: ") + m_path.c_str());
    }
}

void SysFsDriverACPIPlatformProfile::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_action == "change")
    {
        emit kernelEvent({
            .m_driverName = DRIVER_NAME,
            .m_action = SubsystemEvent::Action::CHANGED,
            .m_DriverSpecificEventType = "change",
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

