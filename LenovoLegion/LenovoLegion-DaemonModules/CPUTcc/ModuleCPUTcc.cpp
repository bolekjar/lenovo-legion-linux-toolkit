// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUTcc.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUIntelTcc.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Intel TCC offset, saved with the settings
 */
class ModuleCPUTcc : public ProviderModule
{
public:

    explicit ModuleCPUTcc(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUIntelTcc>());

        setProvider(std::make_unique<SysFsDataProviderCPUTcc>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 113, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUTcc tcc;

        if (storage.contains("offset")) tcc.set_offset(storage.value("offset").toUInt());

        return serialize(tcc);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto tcc = parse<legion::messages::CPUTcc>(message);

        if (tcc.has_offset()) storage.setValue("offset", tcc.offset());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUTcc,"CPUTcc","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUTcc::dataType,"CPUTccData")
