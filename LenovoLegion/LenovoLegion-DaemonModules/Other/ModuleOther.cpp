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
#include <Drivers/SysFsDriverLegionOther.h>
#include <Drivers/SysFsDriverLegionVpc.h>

#include <Other.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Other settings (touchpad, Win key, Legion WMI gamezone; conservation mode, rapid charge, USB charging, Fn lock, camera power,
 * ACPI device VPC2004; instant boot, flip to start, display overdrive, Legion WMI other mode). Owns notifications
 * KEYLOCK_STATUS_CHANGE (Legion WMI event), VPC_FEATURE_CHANGED and OTHER_FEATURE_CHANGED.
 */
class ModuleOther : public ProviderModule
{
public:

    explicit ModuleOther(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionVpc>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());

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

        /*
         * Written by another client (own writes are blocked by the provider)
         */
        if(event.m_driverName == SysFsDriverLegionVpc::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::VPC_FEATURE_CHANGED);
        }

        /*
         * Features of the other mode of this module only (the power limits of the other mode belong to the power modules)
         */
        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED)
        {
            switch(event.m_DriverSpecificEventType.toInt())
            {
            case SysFsDriverLegionOther::LENOVO_WMI_OTHER_IB_AC:
            case SysFsDriverLegionOther::LENOVO_WMI_OTHER_IB_UPD:
            case SysFsDriverLegionOther::LENOVO_WMI_OTHER_FL_ST:
            case SysFsDriverLegionOther::LENOVO_WMI_OTHER_OD:
                return notification(legion::messages::Notification::OTHER_FEATURE_CHANGED);
            default:
                break;
            }
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::OtherSettings otherSettings;

        if (storage.contains("disable_touchpad")) otherSettings.mutable_touch_pad()->set_current(storage.value("disable_touchpad").toBool());
        if (storage.contains("disable_win_key"))  otherSettings.mutable_win_key()->set_current(storage.value("disable_win_key").toBool());

        // ACPI device VPC2004, camera power is not saved (privacy switch, the state of the firmware wins)
        if (storage.contains("conservation_mode")) otherSettings.mutable_conservation_mode()->set_current(storage.value("conservation_mode").toBool());
        if (storage.contains("rapid_charge"))      otherSettings.mutable_rapid_charge()->set_current(storage.value("rapid_charge").toBool());
        if (storage.contains("usb_charging"))      otherSettings.mutable_usb_charging()->set_current(storage.value("usb_charging").toBool());
        if (storage.contains("fn_lock"))           otherSettings.mutable_fn_lock()->set_current(storage.value("fn_lock").toBool());

        // Legion WMI other mode (the firmware keeps them as well, the saved value wins the same as the other settings)
        if (storage.contains("instant_boot_ac"))     otherSettings.mutable_instant_boot_ac()->set_current(storage.value("instant_boot_ac").toBool());
        if (storage.contains("instant_boot_usb_pd")) otherSettings.mutable_instant_boot_usb_pd()->set_current(storage.value("instant_boot_usb_pd").toBool());
        if (storage.contains("flip_to_start"))       otherSettings.mutable_flip_to_start()->set_current(storage.value("flip_to_start").toBool());
        if (storage.contains("over_drive"))          otherSettings.mutable_over_drive()->set_current(storage.value("over_drive").toBool());

        return serialize(otherSettings);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto otherSettings = parse<legion::messages::OtherSettings>(message);

        if (otherSettings.has_touch_pad() && otherSettings.touch_pad().has_current()) storage.setValue("disable_touchpad", otherSettings.touch_pad().current());
        if (otherSettings.has_win_key() && otherSettings.win_key().has_current())     storage.setValue("disable_win_key", otherSettings.win_key().current());

        if (otherSettings.has_conservation_mode() && otherSettings.conservation_mode().has_current()) storage.setValue("conservation_mode", otherSettings.conservation_mode().current());
        if (otherSettings.has_rapid_charge() && otherSettings.rapid_charge().has_current())           storage.setValue("rapid_charge", otherSettings.rapid_charge().current());
        if (otherSettings.has_usb_charging() && otherSettings.usb_charging().has_current())           storage.setValue("usb_charging", otherSettings.usb_charging().current());
        if (otherSettings.has_fn_lock() && otherSettings.fn_lock().has_current())                     storage.setValue("fn_lock", otherSettings.fn_lock().current());

        if (otherSettings.has_instant_boot_ac() && otherSettings.instant_boot_ac().has_current())         storage.setValue("instant_boot_ac", otherSettings.instant_boot_ac().current());
        if (otherSettings.has_instant_boot_usb_pd() && otherSettings.instant_boot_usb_pd().has_current()) storage.setValue("instant_boot_usb_pd", otherSettings.instant_boot_usb_pd().current());
        if (otherSettings.has_flip_to_start() && otherSettings.flip_to_start().has_current())             storage.setValue("flip_to_start", otherSettings.flip_to_start().current());
        if (otherSettings.has_over_drive() && otherSettings.over_drive().has_current())                   storage.setValue("over_drive", otherSettings.over_drive().current());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleOther,"Other","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderOther::dataType,"OtherData")
