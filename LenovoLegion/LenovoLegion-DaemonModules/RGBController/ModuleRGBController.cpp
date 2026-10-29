// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderRGBController.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionEvents.h>

namespace LenovoLegionDaemon::Kit {

/*
 * RGB keyboard (HID, the only part of the daemon which uses hidapi).
 * Reacts to Legion WMI utility events (keyboard backlight keys) and owns notification SPECIAL_KEY_PRESSED.
 */
class ModuleRGBController : public ProviderModule
{
public:

    explicit ModuleRGBController(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());

        setProvider(std::make_unique<DataProviderRGBController>());
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LENOVO_WMI_EVENT_UTILITY)
        {
            auto message = notification(legion::messages::Notification::SPECIAL_KEY_PRESSED);

            message.set_special_key(static_cast<legion::messages::Notification_SpecialKey>(event.m_DriverSpecificEventValue.toUInt()));

            return message;
        }

        return {};
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleRGBController,"RGBController","1.0",LenovoLegionDaemon::Kit::DataProviderRGBController::dataType,nullptr)
