// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderIGPUFrequency.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverIGpuFrequency.h>

#include <NvidiaNvml.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Frequency of the Intel integrated GPU, saved with the settings
 */
class ModuleIGPUFrequency : public ProviderModule
{
public:

    explicit ModuleIGPUFrequency(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverIGpuFrequency>());

        setProvider(std::make_unique<SysFsDataProviderIGPUFrequency>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 114, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::IGpuFrequency frequency;

        if (storage.contains("min_freq"))   frequency.set_min_freq(storage.value("min_freq").toUInt());
        if (storage.contains("max_freq"))   frequency.set_max_freq(storage.value("max_freq").toUInt());
        if (storage.contains("boost_freq")) frequency.set_boost_freq(storage.value("boost_freq").toUInt());

        return serialize(frequency);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto frequency = parse<legion::messages::IGpuFrequency>(message);

        if (frequency.has_min_freq())   storage.setValue("min_freq", frequency.min_freq());
        if (frequency.has_max_freq())   storage.setValue("max_freq", frequency.max_freq());
        if (frequency.has_boost_freq()) storage.setValue("boost_freq", frequency.boost_freq());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleIGPUFrequency,"IGPUFrequency","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderIGPUFrequency::dataType,"IGPUFrequencyData")
