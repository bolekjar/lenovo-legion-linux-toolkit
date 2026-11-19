// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDriverLegionLegacy.h"

#include "../KitLog.h"

namespace LenovoLegionDaemon::Kit {

SysFsDriverLegionLegacy::SysFsDriverLegionLegacy(const char* driverName,const char* sysName) :
    SysFsDriver(driverName,std::filesystem::path("/sys/class/legion-firmware-attributes/").append(sysName).append("attributes/"),{SUBSYSTEM,{"EVENT_TYPE","EVENT_VALUE"}},MODULE_NAME),
    m_sysName(sysName)
{}


void SysFsDriverLegionLegacy::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(!std::filesystem::is_directory(m_path))
    {
        return;
    }

    /*
     * Only the limits supported by the firmware are registered by the kernel driver, default_value is optional
     */
    for(const auto& entry : std::filesystem::directory_iterator(m_path))
    {
        if(!entry.is_directory())
        {
            continue;
        }

        const QString limit = QString::fromStdString(entry.path().filename().string());

        LOG_D(QString("Found Legion legacy limit in path: ") + entry.path().c_str());

        for(const char* file : {"current_value","default_value","min_value","max_value","scalar_increment","supported"})
        {
            const std::filesystem::path path = std::filesystem::path(entry.path()).append(file);

            if(std::filesystem::exists(path))
            {
                m_descriptor[limit + "_" + file] = path;
            }
        }
    }
}

void SysFsDriverLegionLegacy::handleKernelEvent(const KernelEvent::Event &event)
{
    /*
     * The subsystem (class) is shared by all firmware attribute devices of the Lenovo driver
     */
    if(event.m_sysName != m_sysName)
    {
        return;
    }

    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(event.m_action == "add")
    {
        init();
        return;
    }

    if(event.m_action == "remove")
    {
        clean();
        return;
    }

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_action == "change")
    {
        emit kernelEvent({
            .m_driverName = m_name,
            .m_action = SubsystemEvent::Action::CHANGED,
            .m_DriverSpecificEventType = event.m_properties.value("EVENT_TYPE", ""),
            .m_DriverSpecificEventValue = event.m_properties.value("EVENT_VALUE", "")
        });
    }
}

}
