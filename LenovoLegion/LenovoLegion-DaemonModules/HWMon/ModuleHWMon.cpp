// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderHWMon.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUXList.h>
#include <Drivers/SysFsDriverIntelPowercapRapl.h>
#include <Drivers/SysFsDriverLegionHWMon.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Hardware monitoring (temperatures, fans, CPU frequencies, RAPL power)
 */
class ModuleHWMon : public ProviderModule
{
public:

    explicit ModuleHWMon(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUXList>());
        drivers().addDriver(std::make_unique<SysFsDriverIntelPowercapRapl>());
        drivers().addDriver(std::make_unique<SysFsDriverLegionHWMon>());

        setProvider(std::make_unique<SysFsDataProviderHWMon>(&drivers()));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleHWMon,"HWMon","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderHWMon::dataType,nullptr)
