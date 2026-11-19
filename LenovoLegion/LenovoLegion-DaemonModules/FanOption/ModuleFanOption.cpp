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

#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionFanMode.h>
#include <Drivers/SysFsDriverLegionOther.h>

#include <KitLog.h>
#include <LastRequest.h>

#include <FanControl.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Fan options (full speed, Legion WMI other, on the older firmware the fan mode). Owns notification FAN_CURVE_CHANGED
 * for the attribute it writes (fan full speed).
 */
class ModuleFanOption : public ProviderModule
{
public:

    explicit ModuleFanOption(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionFanMode>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());

        setProvider(std::make_unique<SysFsDataProviderFanOption>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 60, .customOnly = false };
    }

    QByteArray set(const QByteArray& request) override
    {
        m_lastRequest.remember(request);

        return ProviderModule::set(request);
    }

    void loadSettings(const Module::ISettingsStorage& storage) override
    {
        m_lastRequest.remember(readSettings(storage));

        ProviderModule::loadSettings(storage);
    }

protected:

    /*
     * Older firmware: the fan full speed is written again when the custom mode is set (the same as God mode of
     * Lenovo Legion Toolkit)
     */
    void onDriverEvent(const SysFsDriver::SubsystemEvent& event) override
    {
        if(SysFsDriverLegionEvents::thermalMode(event) != SysFsDriverLegionEvents::THERMAL_MODE_CUSTOM || m_lastRequest.value().isEmpty() ||
           drivers().isLoaded(SysFsDriverLegionOther::DRIVER_NAME) || !drivers().isLoaded(SysFsDriverLegionFanMode::DRIVER_NAME))
        {
            return;
        }

        try {
            provider().deserializeAndSetData(m_lastRequest.value());
        }
        catch(const std::exception& ex)
        {
            LOG_W(QString("Fan full speed of the custom mode not written again: ").append(ex.what()));
        }
    }

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionOther::LENOVO_WMI_OTHER_FF_S)
        {
            return notification(legion::messages::Notification::FAN_CURVE_CHANGED);
        }

        /*
         * Older firmware: fan full speed of the fan mode
         */
        if(event.m_driverName == SysFsDriverLegionFanMode::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionFanMode::LENOVO_WMI_FAN_FULL_SPEED)
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

        if (fanOption.has_full_speed()) storage.setValue("full_speed", fanOption.full_speed().current_value());
    }

private:

    LastRequest<legion::messages::FanOption> m_lastRequest;
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleFanOption,"FanOption","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderFanOption::dataType,"FanOptionData")
