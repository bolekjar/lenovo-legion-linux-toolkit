// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUUncore.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUIntelUncore.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Intel uncore frequency (min / max), saved with the settings
 */
class ModuleCPUUncore : public ProviderModule
{
public:

    explicit ModuleCPUUncore(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUIntelUncore>());

        setProvider(std::make_unique<SysFsDataProviderCPUUncore>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 112, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUUncore uncore;

        if (storage.contains("min_freq")) uncore.set_min_freq(storage.value("min_freq").toUInt());
        if (storage.contains("max_freq")) uncore.set_max_freq(storage.value("max_freq").toUInt());

        return serialize(uncore);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto uncore = parse<legion::messages::CPUUncore>(message);

        if (uncore.has_min_freq()) storage.setValue("min_freq", uncore.min_freq());
        if (uncore.has_max_freq()) storage.setValue("max_freq", uncore.max_freq());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUUncore,"CPUUncore","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUUncore::dataType,"CPUUncoreData")
