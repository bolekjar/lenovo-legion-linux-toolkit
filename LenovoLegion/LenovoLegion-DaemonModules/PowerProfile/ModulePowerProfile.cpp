// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderPowerProfile.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionGameZone.h>
#include <Drivers/SysFsDriverLegionOther.h>

#include <PowerProfile.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Power profile (thermal mode, Legion WMI gamezone/other). Owns notification THERMAL_MODE_CHANGE
 * (Legion WMI event, e.g. Fn+Q), its settings are restored first.
 */
class ModulePowerProfile : public ProviderModule
{
public:

    explicit ModulePowerProfile(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());

        setProvider(std::make_unique<SysFsDataProviderPowerProfile>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 0, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LEGION_WMI_EVENT_THERMAL_MODE)
        {
            return notification(legion::messages::Notification::THERMAL_MODE_CHANGE);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::PowerProfile profile;

        if (storage.contains("PowerProfile"))       profile.set_current_value(static_cast<legion::messages::PowerProfile::Profiles>(storage.value("PowerProfile").toUInt()));
        if (storage.contains("custom_fnq_enabled")) profile.mutable_custom_fnq_enabled()->set_current_value(storage.value("custom_fnq_enabled").toBool());

        return serialize(profile);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto profile = parse<legion::messages::PowerProfile>(message);

        storage.setValue("PowerProfile", static_cast<quint32>(profile.current_value()));
        storage.setValue("custom_fnq_enabled", profile.custom_fnq_enabled().current_value());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModulePowerProfile,"PowerProfile","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderPowerProfile::dataType,"PowerProfileData")
