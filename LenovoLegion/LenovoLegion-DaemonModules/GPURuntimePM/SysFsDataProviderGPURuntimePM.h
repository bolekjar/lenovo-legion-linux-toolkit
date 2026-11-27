// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <MessageRegistry.pb.h>
#include <NvidiaNvml.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Runtime power management of the discrete NVIDIA GPU: the state (active / suspended) and power/control (auto / on)
 */
class SysFsDataProviderGPURuntimePM : public SysFsDataProvider
{
public:

    explicit SysFsDataProviderGPURuntimePM(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;
    QByteArray settableData(const QByteArray& data,Origin origin) override;

    static constexpr quint8  dataType = legion::messages::DataType::GPU_RUNTIME_PM;

private:

    void settable(legion::messages::GpuRuntimePM& pm,Origin origin) const;
};

}
