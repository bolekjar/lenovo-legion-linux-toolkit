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
#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * Intel TCC offset: the CPU throttles at TjMax - offset
 */
class SysFsDataProviderCPUTcc : public SysFsDataProvider
{
public:

    explicit SysFsDataProviderCPUTcc(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;
    QByteArray settableData(const QByteArray& data,Origin origin) override;

    static constexpr quint8  dataType = legion::messages::DataType::CPU_TCC;

private:

    legion::messages::CPUTcc read() const;
    void settable(legion::messages::CPUTcc& tcc,Origin origin) const;

    /*
     * Offset field of the hardware (6 bits) and the lowest throttling temperature offered
     */
    static constexpr uint32_t MAX_OFFSET       = 63;
    static constexpr uint32_t MIN_THROTTLING_C = 50;
};

}
