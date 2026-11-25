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

#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverPowerSuplyBattery0.h>

#include <Battery.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Battery (power_supply BAT0, charge mode and power adapter from Legion gamezone).
 * Owns notifications POWER_SUPPLY_BATTERY0_CHANGE and POWER_ADAPTER_LOW_WATTAGE.
 */
class ModuleBattery : public ProviderModule
{
public:

    explicit ModuleBattery(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());
        drivers().addDriver(std::make_unique<SysFsDriverPowerSuplyBattery0>());

        auto provider = std::make_unique<SysFsDataProviderBattery>(&drivers());
        m_battery = provider.get();
        setProvider(std::move(provider));
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        const bool powerSupply = event.m_driverName == SysFsDriverPowerSuplyBattery0::DRIVER_NAME &&
                                 event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED;
        const bool acPd        = event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME &&
                                 event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
                                 eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LEGION_WMI_EVENT_AC_PD;

        if(!powerSupply && !acPd)
        {
            return {};
        }

        /*
         * A low wattage adapter connected now (not before): the user is told once, the GUI shows it with the battery
         */
        const auto adapter = m_battery->powerAdapter();
        const bool lowNow  = adapter == legion::messages::Battery::POWER_ADAPTER_CONNECTED_LOW_WATTAGE &&
                             m_lastAdapter != legion::messages::Battery::POWER_ADAPTER_CONNECTED_LOW_WATTAGE;

        m_lastAdapter = adapter;

        return notification(lowNow ? legion::messages::Notification::POWER_ADAPTER_LOW_WATTAGE
                                   : legion::messages::Notification::POWER_SUPPLY_BATTERY0_CHANGE);
    }

private:

    SysFsDataProviderBattery*               m_battery     = nullptr;   // owned by the module (setProvider)
    legion::messages::Battery::PowerAdapter m_lastAdapter = legion::messages::Battery::POWER_ADAPTER_UNKNOWN;
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleBattery,"Battery","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderBattery::dataType,nullptr)
