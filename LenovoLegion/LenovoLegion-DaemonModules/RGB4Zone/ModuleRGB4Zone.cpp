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

#include <Drivers/SysFsDriverLegionGameZone.h>

#include <RGBController.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * 4-zone RGB keyboard (HID). The keyboard can not be read: its state is saved with the settings and set again when the
 * daemon starts.
 */
class ModuleRGB4Zone : public ProviderModule
{
public:

    explicit ModuleRGB4Zone(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());

        setProvider(std::make_unique<DataProviderRGB4Zone>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 200, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::Keyboard4Zone keyboard;

        if (storage.contains("effect"))     keyboard.set_effect(static_cast<legion::messages::Keyboard4Zone::Effect>(storage.value("effect").toInt()));
        if (storage.contains("speed"))      keyboard.set_speed(storage.value("speed").toUInt());
        if (storage.contains("brightness")) keyboard.set_brightness(static_cast<legion::messages::Keyboard4Zone::Brightness>(storage.value("brightness").toInt()));
        if (storage.contains("direction"))  keyboard.set_direction(static_cast<legion::messages::Keyboard4Zone::Direction>(storage.value("direction").toInt()));

        if (storage.contains("zone1_color")) keyboard.set_zone1_color(storage.value("zone1_color").toString().toUInt(nullptr,16));
        if (storage.contains("zone2_color")) keyboard.set_zone2_color(storage.value("zone2_color").toString().toUInt(nullptr,16));
        if (storage.contains("zone3_color")) keyboard.set_zone3_color(storage.value("zone3_color").toString().toUInt(nullptr,16));
        if (storage.contains("zone4_color")) keyboard.set_zone4_color(storage.value("zone4_color").toString().toUInt(nullptr,16));

        return serialize(keyboard);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto keyboard = parse<legion::messages::Keyboard4Zone>(message);

        if (keyboard.has_effect())     storage.setValue("effect", static_cast<int>(keyboard.effect()));
        if (keyboard.has_speed())      storage.setValue("speed", keyboard.speed());
        if (keyboard.has_brightness()) storage.setValue("brightness", static_cast<int>(keyboard.brightness()));
        if (keyboard.has_direction())  storage.setValue("direction", static_cast<int>(keyboard.direction()));

        // Colors as 0xRRGGBB text (readable in the settings file)
        if (keyboard.has_zone1_color()) storage.setValue("zone1_color", QString::number(keyboard.zone1_color(),16));
        if (keyboard.has_zone2_color()) storage.setValue("zone2_color", QString::number(keyboard.zone2_color(),16));
        if (keyboard.has_zone3_color()) storage.setValue("zone3_color", QString::number(keyboard.zone3_color(),16));
        if (keyboard.has_zone4_color()) storage.setValue("zone4_color", QString::number(keyboard.zone4_color(),16));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleRGB4Zone,"RGB4Zone","1.0",LenovoLegionDaemon::Kit::DataProviderRGB4Zone::dataType,"RGB4ZoneData")
