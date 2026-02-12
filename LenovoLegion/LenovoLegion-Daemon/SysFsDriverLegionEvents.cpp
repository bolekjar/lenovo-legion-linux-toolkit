// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverLegionEvents.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

SysFsDriverLegionEvents::SysFsDriverLegionEvents(QObject *parrent) :SysFsDriver(DRIVER_NAME,"",{"wmi",{EVENT_TYPE.data(),EVENT_VALUE.data()}},parrent) {}

void SysFsDriverLegionEvents::init()
{}

void SysFsDriverLegionEvents::handleKernelEvent(const SysFsDriver::KernelEvent::Event &event)
{
     LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath + ", " +
                                                                         EVENT_TYPE.data() + "=" + event.m_properties[EVENT_TYPE.data()] + ", " + EVENT_VALUE.data() + "=" + event.m_properties[EVENT_VALUE.data()]);


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
             .m_DriverSpecificEventType = event.m_properties[EVENT_TYPE.data()],
             .m_DriverSpecificEventValue  = event.m_properties[EVENT_VALUE.data()]
         });
     }
}

}
