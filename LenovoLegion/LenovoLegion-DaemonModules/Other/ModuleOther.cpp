// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderOther.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionGameZone.h>

#include <Other.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Other settings (touchpad, Win key, Legion WMI gamezone). Owns notification KEYLOCK_STATUS_CHANGE (Legion WMI event).
 */
class ModuleOther : public ProviderModule
{
public:

    explicit ModuleOther(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());

        setProvider(std::make_unique<SysFsDataProviderOther>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 100, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
           eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LEGION_WMI_EVENT_KEYLOCK_STATUS)
        {
            auto message = notification(legion::messages::Notification::KEYLOCK_STATUS_CHANGE);

            message.set_key_lock_key(static_cast<legion::messages::Notification_KeylockDisabledBit>(event.m_DriverSpecificEventValue.toUInt()));

            return message;
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::OtherSettings otherSettings;

        if (storage.contains("disable_touchpad")) otherSettings.mutable_touch_pad()->set_current(storage.value("disable_touchpad").toBool());
        if (storage.contains("disable_win_key"))  otherSettings.mutable_win_key()->set_current(storage.value("disable_win_key").toBool());

        return serialize(otherSettings);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto otherSettings = parse<legion::messages::OtherSettings>(message);

        if (otherSettings.has_touch_pad() && otherSettings.touch_pad().has_current()) storage.setValue("disable_touchpad", otherSettings.touch_pad().current());
        if (otherSettings.has_win_key() && otherSettings.win_key().has_current())     storage.setValue("disable_win_key", otherSettings.win_key().current());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleOther,"Other","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderOther::dataType,"OtherData")
