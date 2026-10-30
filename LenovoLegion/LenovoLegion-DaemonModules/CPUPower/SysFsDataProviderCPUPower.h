// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <CpuPower.pb.h>
#include <MessageRegistry.pb.h>
#include <PowerProfile.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderCPUPower : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUPower(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    ;
    virtual QByteArray deserializeAndSetData(const QByteArray&) ;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::CPUPower& cpuPower,Origin origin);

    void setValue(const std::filesystem::path& path,std::function<void (legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value)> setter,auto map) const;
    void setValuesSteps(const std::filesystem::path& path,std::function<void (legion::messages::CPUPower::Limit::Descriptor &descriptor,const QList<QString>& values)> setter,auto map) const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_POWER;
};

}
