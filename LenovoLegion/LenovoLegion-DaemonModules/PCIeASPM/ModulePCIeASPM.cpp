// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderPCIeASPM.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverPcieAspm.h>

#include <Other.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * PCIe ASPM policy of the kernel, saved with the settings
 */
class ModulePCIeASPM : public ProviderModule
{
public:

    explicit ModulePCIeASPM(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverPcieAspm>());

        setProvider(std::make_unique<SysFsDataProviderPCIeASPM>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 115, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::PcieAspm aspm;

        if (storage.contains("policy")) aspm.set_policy(storage.value("policy").toString().toStdString());

        return serialize(aspm);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto aspm = parse<legion::messages::PcieAspm>(message);

        if (aspm.has_policy()) storage.setValue("policy", QString::fromStdString(std::string(aspm.policy())));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModulePCIeASPM,"PCIeASPM","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderPCIeASPM::dataType,"PCIeASPMData")
