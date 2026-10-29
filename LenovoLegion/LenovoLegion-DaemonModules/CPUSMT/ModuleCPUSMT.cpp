// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUSMT.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPU.h>
#include <Drivers/SysFsDriverCPUXList.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU SMT control
 */
class ModuleCPUSMT : public ProviderModule
{
public:

    explicit ModuleCPUSMT(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPU>());
        drivers().addDriver(std::make_unique<SysFsDriverCPUXList>());

        setProvider(std::make_unique<SysFsDataProviderCPUSMT>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 70, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUSMT smt;

        if (storage.contains("control")) smt.set_control(storage.value("control").toString().toStdString());

        return serialize(smt);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto smt = parse<legion::messages::CPUSMT>(message);

        storage.setValue("control", QString::fromStdString(std::string(smt.control())));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUSMT,"CPUSMT","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUSMT::dataType,"CPUSMTData")
