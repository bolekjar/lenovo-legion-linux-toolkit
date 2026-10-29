// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUTopology.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPU.h>
#include <Drivers/SysFsDriverCPUAtom.h>
#include <Drivers/SysFsDriverCPUCore.h>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU topology (performance/efficient cores), no settings
 */
class ModuleCPUTopology : public ProviderModule
{
public:

    explicit ModuleCPUTopology(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPU>());
        drivers().addDriver(std::make_unique<SysFsDriverCPUAtom>());
        drivers().addDriver(std::make_unique<SysFsDriverCPUCore>());

        setProvider(std::make_unique<SysFsDataProviderCPUTopology>(&drivers()));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUTopology,"CPUTopology","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUTopology::dataType,nullptr)
