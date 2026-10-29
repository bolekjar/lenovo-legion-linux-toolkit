// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderGPUPower.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <Drivers/SysFsDriverLegionOther.h>

#include <GPUPower.pb.h>

#include <set>

namespace LenovoLegionDaemon::Kit {

/*
 * GPU power limits (Legion WMI other). Owns notification POWER_CONTROL_CHANGED
 * for the attributes it writes (GPU power boost, configurable TGP, temperature limit).
 */
class ModuleGPUPower : public ProviderModule
{
public:

    explicit ModuleGPUPower(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        drivers().addDriver(std::make_unique<SysFsDriverLegionOther>());

        setProvider(std::make_unique<SysFsDataProviderGPUPower>(&drivers()));
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 50, .customOnly = true };
    }

protected:

    std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent& event) override
    {
        static const std::set<int> OWNED {
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_GPU_PB,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_GPU_TGP,
            SysFsDriverLegionOther::LENOVO_WMI_OTHER_GPU_TMP
        };

        if(event.m_driverName == SysFsDriverLegionOther::DRIVER_NAME && event.m_action == SysFsDriver::SubsystemEvent::Action::CHANGED && OWNED.contains(eventType(event)))
        {
            return notification(legion::messages::Notification::POWER_CONTROL_CHANGED);
        }

        return {};
    }

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::GPUPower gpuPower;

        if (storage.contains("gpu_power_boost"))       gpuPower.mutable_gpu_power_boost()->set_current_value(storage.value("gpu_power_boost").toUInt());
        if (storage.contains("gpu_configurable_tgp"))  gpuPower.mutable_gpu_configurable_tgp()->set_current_value(storage.value("gpu_configurable_tgp").toUInt());
        if (storage.contains("gpu_temperature_limit")) gpuPower.mutable_gpu_temperature_limit()->set_current_value(storage.value("gpu_temperature_limit").toUInt());

        return serialize(gpuPower);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto gpuPower = parse<legion::messages::GPUPower>(message);

        if (gpuPower.has_gpu_power_boost())       storage.setValue("gpu_power_boost", gpuPower.gpu_power_boost().current_value());
        if (gpuPower.has_gpu_configurable_tgp())  storage.setValue("gpu_configurable_tgp", gpuPower.gpu_configurable_tgp().current_value());
        if (gpuPower.has_gpu_temperature_limit()) storage.setValue("gpu_temperature_limit", gpuPower.gpu_temperature_limit().current_value());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleGPUPower,"GPUPower","1.0",LenovoLegionDaemon::Kit::SysFsDataProviderGPUPower::dataType,"GPUPowerData")
