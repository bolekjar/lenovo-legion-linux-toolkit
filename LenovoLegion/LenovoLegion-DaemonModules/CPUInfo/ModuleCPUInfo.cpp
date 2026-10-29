// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUInfo.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUInfo.h>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU information (cpuid, cpufreq info)
 */
class ModuleCPUInfo : public ProviderModule
{
public:

    explicit ModuleCPUInfo(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUInfo>());

        setProvider(std::make_unique<SysFsDataProviderCPUInfo>(&drivers()));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUInfo,"CPUInfo","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUInfo::dataType,nullptr)
