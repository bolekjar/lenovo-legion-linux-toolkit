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
#include <Other.pb.h>

namespace LenovoLegionDaemon::Kit {

/*
 * PCIe ASPM policy of the kernel: the current one ([...]) and the offered ones
 */
class SysFsDataProviderPCIeASPM : public SysFsDataProvider
{
public:

    explicit SysFsDataProviderPCIeASPM(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;
    QByteArray settableData(const QByteArray& data,Origin origin) override;

    static constexpr quint8  dataType = legion::messages::DataType::PCIE_ASPM;

private:

    legion::messages::PcieAspm read() const;
    void settable(legion::messages::PcieAspm& aspm,Origin origin) const;
};

}
