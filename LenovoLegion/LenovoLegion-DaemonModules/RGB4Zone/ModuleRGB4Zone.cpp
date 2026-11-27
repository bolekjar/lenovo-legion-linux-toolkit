// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderRGB4Zone.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionEvents.h>
#include <Drivers/SysFsDriverLegionGameZone.h>

#include <RGBController.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * 4-zone RGB keyboard (HID). The keyboard can not be read: the presets and the active one are saved with the settings
 * and set again when the daemon starts. Fn+Space (light profile change event of the firmware) switches the preset,
 * owns notification KEYBOARD_FOUR_ZONE_CHANGED.
 */
class ModuleRGB4Zone : public ProviderModule
{
public:

    explicit ModuleRGB4Zone(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionEvents>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());

        auto provider = std::make_unique<DataProviderRGB4Zone>(&drivers());
        m_keyboard = provider.get();
        setProvider(std::move(provider));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 200, .customOnly = false };
    }

protected:

    static bool lightProfileEvent(const SysFsDriver::SubsystemEvent& event)
    {
        return event.m_driverName == SysFsDriverLegionEvents::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED &&
               eventType(event) == SysFsDriverLegionEvents::LegionVmiEventType::LEGION_WMI_EVENT_LIGHT_PROFILE;
    }

    void onDriverEvent(const SysFsDriver::SubsystemEvent& event) override
    {
        if(lightProfileEvent(event))
        {
            m_switched = m_keyboard->nextPreset();
        }
    }

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(lightProfileEvent(event) && m_switched)
        {
            m_switched = false;
            return notification(legion::messages::Notification::KEYBOARD_FOUR_ZONE_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::Keyboard4Zone keyboard;

        if (storage.contains("active_preset")) keyboard.set_active_preset(storage.value("active_preset").toUInt());

        for (int i = 0; i < DataProviderRGB4Zone::PRESETS; ++i)
        {
            const QString                            prefix = QString("preset%1/").arg(i + 1);
            legion::messages::Keyboard4Zone::Preset* preset = keyboard.add_presets();

            if (storage.contains(prefix + "effect"))      preset->set_effect(static_cast<legion::messages::Keyboard4Zone::Effect>(storage.value(prefix + "effect").toInt()));
            if (storage.contains(prefix + "speed"))       preset->set_speed(storage.value(prefix + "speed").toUInt());
            if (storage.contains(prefix + "brightness"))  preset->set_brightness(static_cast<legion::messages::Keyboard4Zone::Brightness>(storage.value(prefix + "brightness").toInt()));
            if (storage.contains(prefix + "direction"))   preset->set_direction(static_cast<legion::messages::Keyboard4Zone::Direction>(storage.value(prefix + "direction").toInt()));
            if (storage.contains(prefix + "zone1_color")) preset->set_zone1_color(storage.value(prefix + "zone1_color").toString().toUInt(nullptr,16));
            if (storage.contains(prefix + "zone2_color")) preset->set_zone2_color(storage.value(prefix + "zone2_color").toString().toUInt(nullptr,16));
            if (storage.contains(prefix + "zone3_color")) preset->set_zone3_color(storage.value(prefix + "zone3_color").toString().toUInt(nullptr,16));
            if (storage.contains(prefix + "zone4_color")) preset->set_zone4_color(storage.value(prefix + "zone4_color").toString().toUInt(nullptr,16));
        }

        /*
         * Nothing saved: no request (the defaults of the provider are not written)
         */
        if (!keyboard.has_active_preset())
        {
            return {};
        }

        return serialize(keyboard);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto keyboard = parse<legion::messages::Keyboard4Zone>(message);

        if (keyboard.has_active_preset()) storage.setValue("active_preset", keyboard.active_preset());

        for (int i = 0; i < keyboard.presets_size(); ++i)
        {
            const QString prefix = QString("preset%1/").arg(i + 1);
            const auto&   preset = keyboard.presets(i);

            if (preset.has_effect())      storage.setValue(prefix + "effect", static_cast<int>(preset.effect()));
            if (preset.has_speed())       storage.setValue(prefix + "speed", preset.speed());
            if (preset.has_brightness())  storage.setValue(prefix + "brightness", static_cast<int>(preset.brightness()));
            if (preset.has_direction())   storage.setValue(prefix + "direction", static_cast<int>(preset.direction()));

            // Colors as RRGGBB text (readable in the settings file)
            if (preset.has_zone1_color()) storage.setValue(prefix + "zone1_color", QString::number(preset.zone1_color(),16));
            if (preset.has_zone2_color()) storage.setValue(prefix + "zone2_color", QString::number(preset.zone2_color(),16));
            if (preset.has_zone3_color()) storage.setValue(prefix + "zone3_color", QString::number(preset.zone3_color(),16));
            if (preset.has_zone4_color()) storage.setValue(prefix + "zone4_color", QString::number(preset.zone4_color(),16));
        }
    }

private:

    DataProviderRGB4Zone* m_keyboard = nullptr;   // owned by the module (setProvider)
    bool                  m_switched = false;     // the last light profile event switched the preset
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleRGB4Zone,"RGB4Zone","1.0",LenovoLegionDaemon::Kit::DataProviderRGB4Zone::dataType,"RGB4ZoneData")
