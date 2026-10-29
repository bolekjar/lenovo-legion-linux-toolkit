// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderOtherGpuSwitch.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionGameZone.h>

namespace LenovoLegionDaemon::Kit {

/*
 * GPU switch (hybrid/discrete, Legion WMI gamezone), no settings
 */
class ModuleOtherGpuSwitch : public ProviderModule
{
public:

    explicit ModuleOtherGpuSwitch(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionGameZone>());

        setProvider(std::make_unique<SysFsDataProviderOtherGpuSwitch>(&drivers()));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleOtherGpuSwitch,"OtherGpuSwitch","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderOtherGpuSwitch::dataType,nullptr)
