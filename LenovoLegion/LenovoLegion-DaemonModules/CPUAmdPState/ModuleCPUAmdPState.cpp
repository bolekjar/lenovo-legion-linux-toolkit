// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUAmdPState.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUAmdPState.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * AMD P-state options. Owns notification CPU_AMD_PSTATE_CHANGED (periodic check of the provider).
 */
class ModuleCPUAmdPState : public ProviderModule
{
public:

    explicit ModuleCPUAmdPState(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUAmdPState>());

        setProvider(std::make_unique<SysFsDataProviderCPUAmdPState>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 111, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const DataProvider::SubsystemEvent& event) override
    {
        if(event.m_action == DataProvider::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::CPU_AMD_PSTATE_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUAmdPState pstate;

        if (storage.contains("status")) pstate.set_status(storage.value("status").toString().toStdString());
        if (storage.contains("boost"))  pstate.set_boost(storage.value("boost").toBool());
        if (storage.contains("epp"))    pstate.set_energy_performance_preference(storage.value("epp").toString().toStdString());

        return serialize(pstate);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto pstate = parse<legion::messages::CPUAmdPState>(message);

        if (pstate.has_status())                        storage.setValue("status", QString::fromStdString(std::string(pstate.status())));
        if (pstate.has_boost())                         storage.setValue("boost", pstate.boost());
        if (pstate.has_energy_performance_preference()) storage.setValue("epp", QString::fromStdString(std::string(pstate.energy_performance_preference())));
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUAmdPState,"CPUAmdPState","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUAmdPState::dataType,"CPUAmdPStateData")
