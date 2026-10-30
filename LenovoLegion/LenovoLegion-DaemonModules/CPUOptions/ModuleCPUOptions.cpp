// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUOptions.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverCPUXList.h>

#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU options (online, governor per CPU). Owns notification CPU_X_LIST_RELOADED (it writes the online state,
 * own changes are blocked, changes of SMT by the CPUSMT module are notified too).
 */
class ModuleCPUOptions : public ProviderModule
{
public:

    static constexpr const char* ARRAY = "CPUControl";

    explicit ModuleCPUOptions(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverCPUXList>());

        setProvider(std::make_unique<SysFsDataProviderCPUOptions>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 10, .customOnly = false };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        if(event.m_driverName == SysFsDriverCPUXList::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::RELOADED)
        {
            return notification(legion::messages::Notification::CPU_X_LIST_RELOADED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUOptions options;
        const int                    size = storage.value(arraySizeKey(ARRAY),0).toInt();

        for(int i = 0; i < size; ++i)
        {
            auto* cpux = options.add_cpus();

            if(storage.contains(arrayKey(ARRAY,i,"online")))   cpux->set_cpu_online(storage.value(arrayKey(ARRAY,i,"online")).toBool());
            if(storage.contains(arrayKey(ARRAY,i,"governor"))) cpux->set_governor(storage.value(arrayKey(ARRAY,i,"governor")).toString().toStdString());
        }

        return serialize(options);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto options = parse<legion::messages::CPUOptions>(message);

        for(int i = 0; i < options.cpus().size(); ++i)
        {
            if(options.cpus().at(i).has_cpu_online()) storage.setValue(arrayKey(ARRAY,i,"online"),   options.cpus().at(i).cpu_online());
            if(options.cpus().at(i).has_governor())   storage.setValue(arrayKey(ARRAY,i,"governor"), QString::fromStdString(std::string(options.cpus().at(i).governor())));
        }

        if(options.cpus().size() > 0)
        {
            storage.setValue(arraySizeKey(ARRAY),options.cpus().size());
        }
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUOptions,"CPUOptions","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUOptions::dataType,"CPUsControlData")
