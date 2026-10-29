// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUFrequency.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUXList.h>

#include <CPUFrequency.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU frequency limits per CPU. Owns notification CPU_FREQUENCY_CHANGED (periodic check of the provider).
 */
class ModuleCPUFrequency : public ProviderModule
{
public:

    static constexpr const char* ARRAY = "CPUFrequency";

    explicit ModuleCPUFrequency(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUXList>());

        setProvider(std::make_unique<SysFsDataProviderCPUFrequency>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 20, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const DataProvider::SubsystemEvent& event) override
    {
        if(event.m_action == DataProvider::SubsystemEvent::Action::CHANGED)
        {
            return notification(legion::messages::Notification::CPU_FREQUENCY_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUFrequency frequency;
        const int                      size = storage.value(arraySizeKey(ARRAY),0).toInt();

        for(int i = 0; i < size; ++i)
        {
            auto* cpux = frequency.add_cpus();

            if(storage.contains(arrayKey(ARRAY,i,"scaling_min_freq"))) cpux->set_scaling_min_freq(storage.value(arrayKey(ARRAY,i,"scaling_min_freq")).toUInt());
            if(storage.contains(arrayKey(ARRAY,i,"scaling_max_freq"))) cpux->set_scaling_max_freq(storage.value(arrayKey(ARRAY,i,"scaling_max_freq")).toUInt());
        }

        return serialize(frequency);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto frequency = parse<legion::messages::CPUFrequency>(message);

        for(int i = 0; i < frequency.cpus().size(); ++i)
        {
            storage.setValue(arrayKey(ARRAY,i,"scaling_min_freq"), frequency.cpus().at(i).scaling_min_freq());
            storage.setValue(arrayKey(ARRAY,i,"scaling_max_freq"), frequency.cpus().at(i).scaling_max_freq());
        }

        if(frequency.cpus().size() > 0)
        {
            storage.setValue(arraySizeKey(ARRAY),frequency.cpus().size());
        }
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUFrequency,"CPUFrequency","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUFrequency::dataType,"CPUFrequencyData")
