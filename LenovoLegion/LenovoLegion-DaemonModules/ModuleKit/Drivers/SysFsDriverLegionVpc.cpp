// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDriverLegionVpc.h"

#include "../KitLog.h"

namespace LenovoLegionDaemon::Kit {

SysFsDriverLegionVpc::SysFsDriverLegionVpc() : SysFsDriver(DRIVER_NAME,"/sys/class/legion-firmware-attributes/legion-vpc-0/attributes/",{SUBSYSTEM,{"EVENT_TYPE","EVENT_VALUE"}},MODULE_NAME)
{}


void SysFsDriverLegionVpc::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    for(const char* feature : {"conservation_mode","usb_charging","fn_lock","camera_power"})
    {
        const std::filesystem::path path = std::filesystem::path(m_path).append(feature);

        if(std::filesystem::exists(path))
        {
            LOG_D(QString("Found Legion VPC feature in path: ") + path.c_str());

            m_descriptor[QString(feature) + "_current_value"] = std::filesystem::path(path).append("current_value");
            m_descriptor[QString(feature) + "_supported"]     = std::filesystem::path(path).append("supported");
        }
    }
}

void SysFsDriverLegionVpc::handleKernelEvent(const KernelEvent::Event &event)
{
    /*
     * The subsystem (class) is shared by all firmware attribute devices of the Lenovo driver
     */
    if(event.m_sysName != SYS_NAME)
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
            .m_driverName = DRIVER_NAME,
            .m_action = SubsystemEvent::Action::CHANGED,
            .m_DriverSpecificEventType = event.m_properties.value("EVENT_TYPE", ""),
            .m_DriverSpecificEventValue = event.m_properties.value("EVENT_VALUE", "")
        });
    }
}

}
