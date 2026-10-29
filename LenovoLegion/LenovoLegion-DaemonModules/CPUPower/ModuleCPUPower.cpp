// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUPower.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionOther.h>

#include <CpuPower.pb.h>

#include <set>

namespace LenovoLegionDaemon::Kit {

/*
 * CPU power limits (Legion WMI other). Owns notification POWER_CONTROL_CHANGED
 * for the attributes it writes (CPU limits, APU PPT, GPU total on AC).
 */
class ModuleCPUPower : public ProviderModule
{
public:

    explicit ModuleCPUPower(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());

        setProvider(std::make_unique<SysFsDataProviderCPUPower>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 40, .customOnly = true };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        static const std::set<int> OWNED {
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_STP, SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_LTP,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_PP,  SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_TMP,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_CLP, SysFsDriverLegionOther::LENOVO_WMI_OTHER_CPU_PL1_TAU,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_APU_PPT, SysFsDriverLegionOther::LENOVO_WMI_OTHER_GPU_TAC
        };

        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED && OWNED.contains(eventType(event)))
        {
            return notification(legion::messages::Notification::POWER_CONTROL_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::CPUPower cpuPower;

        if (storage.contains("cpu_stp_limit"))            cpuPower.mutable_cpu_stp_limit()->set_current_value(storage.value("cpu_stp_limit").toUInt());
        if (storage.contains("cpu_ltp_limit"))            cpuPower.mutable_cpu_ltp_limit()->set_current_value(storage.value("cpu_ltp_limit").toUInt());
        if (storage.contains("cpu_clp_limit"))            cpuPower.mutable_cpu_clp_limit()->set_current_value(storage.value("cpu_clp_limit").toUInt());
        if (storage.contains("cpu_tmp_limit"))            cpuPower.mutable_cpu_tmp_limit()->set_current_value(storage.value("cpu_tmp_limit").toUInt());
        if (storage.contains("cpu_pl1_tau"))              cpuPower.mutable_cpu_pl1_tau()->set_current_value(storage.value("cpu_pl1_tau").toUInt());
        if (storage.contains("gpu_total_onac"))           cpuPower.mutable_gpu_total_onac()->set_current_value(storage.value("gpu_total_onac").toUInt());
        if (storage.contains("gpu_to_cpu_dynamic_boost")) cpuPower.mutable_gpu_to_cpu_dynamic_boost()->set_current_value(storage.value("gpu_to_cpu_dynamic_boost").toUInt());

        return serialize(cpuPower);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto cpuPower = parse<legion::messages::CPUPower>(message);

        if (cpuPower.has_cpu_stp_limit())            storage.setValue("cpu_stp_limit", cpuPower.cpu_stp_limit().current_value());
        if (cpuPower.has_cpu_ltp_limit())            storage.setValue("cpu_ltp_limit", cpuPower.cpu_ltp_limit().current_value());
        if (cpuPower.has_cpu_clp_limit())            storage.setValue("cpu_clp_limit", cpuPower.cpu_clp_limit().current_value());
        if (cpuPower.has_cpu_tmp_limit())            storage.setValue("cpu_tmp_limit", cpuPower.cpu_tmp_limit().current_value());
        if (cpuPower.has_cpu_pl1_tau())              storage.setValue("cpu_pl1_tau", cpuPower.cpu_pl1_tau().current_value());
        if (cpuPower.has_gpu_total_onac())           storage.setValue("gpu_total_onac", cpuPower.gpu_total_onac().current_value());
        if (cpuPower.has_gpu_to_cpu_dynamic_boost()) storage.setValue("gpu_to_cpu_dynamic_boost", cpuPower.gpu_to_cpu_dynamic_boost().current_value());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleCPUPower,"CPUPower","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderCPUPower::dataType,"CPUPowerData")
