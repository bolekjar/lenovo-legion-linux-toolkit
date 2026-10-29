// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUPState.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUIntelPState.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Intel P-state options. Owns notification CPU_PSTATE_CHANGED (periodic check of the provider).
 */
class ModuleCPUPState : public ProviderModule
{
public:

    explicit ModuleCPUPState(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUIntelPState>());

        setProvider(std::make_unique<SysFsDataProviderCPUPState>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 110, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const DataProvider::SubsystemEvent& event) override
    {
        if(event.m_action == DataProvider::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::CPU_PSTATE_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUPState pstate;

        if (storage.contains("status"))            pstate.set_status(storage.value("status").toString().toStdString());
        if (storage.contains("hwp_dynamic_boost")) pstate.set_hwp_dynamic_boost(storage.value("hwp_dynamic_boost").toBool());
        if (storage.contains("no_turbo"))          pstate.set_no_turbo(storage.value("no_turbo").toBool());
        if (storage.contains("min_perf_pct"))      pstate.set_min_perf_pct(storage.value("min_perf_pct").toUInt());
        if (storage.contains("max_perf_pct"))      pstate.set_max_perf_pct(storage.value("max_perf_pct").toUInt());

        return serialize(pstate);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto pstate = parse<legion::messages::CPUPState>(message);

        if (pstate.has_status())            storage.setValue("status", QString::fromStdString(std::string(pstate.status())));
        if (pstate.has_hwp_dynamic_boost()) storage.setValue("hwp_dynamic_boost", pstate.hwp_dynamic_boost());
        if (pstate.has_no_turbo())          storage.setValue("no_turbo", pstate.no_turbo());
        if (pstate.has_min_perf_pct())      storage.setValue("min_perf_pct", pstate.min_perf_pct());
        if (pstate.has_max_perf_pct())      storage.setValue("max_perf_pct", pstate.max_perf_pct());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUPState,"CPUPState","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUPState::dataType,"CPUPStateData")
