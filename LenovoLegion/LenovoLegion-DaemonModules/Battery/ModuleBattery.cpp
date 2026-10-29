// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderBattery.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverPowerSuplyBattery0.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Battery (power_supply BAT0, charge mode from Legion gamezone).
 * Owns notification POWER_SUPPLY_BATTERY0_CHANGE.
 */
class ModuleBattery : public ProviderModule
{
public:

    explicit ModuleBattery(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());
        drivers().addDriver(std::make_unique<SysFsDriverPowerSuplyBattery0>());

        setProvider(std::make_unique<SysFsDataProviderBattery>(&drivers()));
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverPowerSuplyBattery0::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::POWER_SUPPLY_BATTERY0_CHANGE);
        }

        return {};
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleBattery,"Battery","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderBattery::dataType,nullptr)
