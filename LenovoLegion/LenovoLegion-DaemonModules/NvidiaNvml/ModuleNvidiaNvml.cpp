// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderNvidiaNvml.h"

#include <ModuleEntry.h>
#include <ProviderModule.h>

#include <NvidiaNvml.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * NVIDIA GPU (NVML, the only part of the daemon which uses libnvidia-ml)
 */
class ModuleNvidiaNvml : public ProviderModule
{
public:

    explicit ModuleNvidiaNvml(Module::IModuleHost& host) :
        ProviderModule(host)
    {
        setProvider(std::make_unique<DataProviderNvidiaNvml>());
    }

    SettingsPolicy settingsPolicy() const override
    {
        return { .persistent = true, .order = 80, .customOnly = false };
    }

protected:

    QByteArray readSettings(const Module::ISettingsStorage& storage) override
    {
        legion::messages::NvidiaNvml nvml;

        if (storage.contains("gpu_offset"))    nvml.mutable_gpu_offset()->set_value(storage.value("gpu_offset").toInt());
        if (storage.contains("memory_offset")) nvml.mutable_memory_offset()->set_value(storage.value("memory_offset").toInt());

        return serialize(nvml);
    }

    void writeSettings(Module::ISettingsStorage& storage,const QByteArray& message) override
    {
        const auto nvml = parse<legion::messages::NvidiaNvml>(message);

        if (nvml.has_gpu_offset())    storage.setValue("gpu_offset", nvml.gpu_offset().value());
        if (nvml.has_memory_offset()) storage.setValue("memory_offset", nvml.memory_offset().value());
    }
};

}

LEGION_KIT_MODULE(LenovoLegionDaemon::Kit::ModuleNvidiaNvml,"NvidiaNvml","1.0",LenovoLegionDaemon::Kit::DataProviderNvidiaNvml::dataType,"NvidiaNvmlData")
