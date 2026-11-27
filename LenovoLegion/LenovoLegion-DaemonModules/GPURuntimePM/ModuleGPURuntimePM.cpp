// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderGPURuntimePM.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverGPURuntimePM.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Runtime power management of the discrete NVIDIA GPU. Not saved: the NVIDIA driver sets it on every start (udev rule),
 * a change in the application is meant for the diagnostics (the GUI warns when it is disabled)
 */
class ModuleGPURuntimePM : public ProviderModule
{
public:

    explicit ModuleGPURuntimePM(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverGPURuntimePM>());

        setProvider(std::make_unique<SysFsDataProviderGPURuntimePM>(&drivers()));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleGPURuntimePM,"GPURuntimePM","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderGPURuntimePM::dataType,nullptr)
