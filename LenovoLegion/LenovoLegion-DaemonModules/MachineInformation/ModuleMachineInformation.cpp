// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderMachineInformation.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegion.h>
#include <Drivers/SysFsDriverLegionMachineInformation.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Machine information (DMI) of the Legion kernel module.
 * Owns notifications LENOVO_DRIVER_ADDED/REMOVED (kernel module lenovo_legion loaded/unloaded).
 */
class ModuleMachineInformation : public ProviderModule
{
public:

    explicit ModuleMachineInformation(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionMachineInformation>());

        setProvider(std::make_unique<SysFsDataProviderMachineInformation>(&drivers()));
    }

protected:

    void onKernelModule(const QString& name,bool loaded) override
    {
        if(name == LEGION_MODULE_NAME)
        {
            notify(notification(loaded ? legion::messages::Notification::LENOVO_DRIVER_ADDED : legion::messages::Notification::LENOVO_DRIVER_REMOVED));
        }
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleMachineInformation,"MachineInformation","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderMachineInformation::dataType,nullptr)
