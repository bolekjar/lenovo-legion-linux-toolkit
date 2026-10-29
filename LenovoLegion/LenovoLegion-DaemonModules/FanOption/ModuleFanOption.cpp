// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderFanOption.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionOther.h>

#include <FanControl.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Fan options (full speed, Legion WMI other). Owns notification FAN_CURVE_CHANGED
 * for the attribute it writes (fan full speed).
 */
class ModuleFanOption : public ProviderModule
{
public:

    explicit ModuleFanOption(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());

        setProvider(std::make_unique<SysFsDataProviderFanOption>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 60, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionOther::LENOVO_WMI_OTHER_FF_S)
        {
            return notification(legion::messages::Notification::FAN_CURVE_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::FanOption fanOption;

        if (storage.contains("full_speed")) fanOption.mutable_full_speed()->set_current_value(storage.value("full_speed").toBool());

        return serialize(fanOption);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto fanOption = parse<legion::messages::FanOption>(message);

        storage.setValue("full_speed", fanOption.full_speed().current_value());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleFanOption,"FanOption","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderFanOption::dataType,"FanOptionData")
