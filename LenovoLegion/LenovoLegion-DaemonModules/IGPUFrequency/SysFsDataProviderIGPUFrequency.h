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
 * Frequency of the Intel integrated GPU (i915 / xe): min, max, boost (i915) in the limits of the hardware
 */
class SysFsDataProviderIGPUFrequency : public SysFsDataProvider
{
public:

    explicit SysFsDataProviderIGPUFrequency(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;
    QByteArray settableData(const QByteArray& data,Origin origin) override;

    static constexpr quint8  dataType = legion::messages::DataType::IGPU_FREQUENCY;

private:

    legion::messages::IGpuFrequency read() const;
    void settable(legion::messages::IGpuFrequency& frequency,Origin origin) const;
};

}
