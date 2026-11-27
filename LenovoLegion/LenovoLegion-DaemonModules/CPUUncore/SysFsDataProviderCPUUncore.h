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
 * Intel uncore frequency: min / max of all domains (packages / dies), the limits of the hardware of the first one
 */
class SysFsDataProviderCPUUncore : public SysFsDataProvider
{
public:

    explicit SysFsDataProviderCPUUncore(SysFsDriverManager* sysFsDriverManager);

    QByteArray serializeAndGetData()                         override;
    QByteArray deserializeAndSetData(const QByteArray &data) override;
    QByteArray settableData(const QByteArray& data,Origin origin) override;

    static constexpr quint8  dataType = legion::messages::DataType::CPU_UNCORE;

private:

    legion::messages::CPUUncore read() const;
    void settable(legion::messages::CPUUncore& uncore,Origin origin) const;
};

}
