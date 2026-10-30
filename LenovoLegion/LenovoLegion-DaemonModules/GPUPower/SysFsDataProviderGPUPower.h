// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <MessageRegistry.pb.h>
#include <GPUPower.pb.h>
#include <PowerProfile.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderGPUPower : public SysFsDataProvider
{
public:

    SysFsDataProviderGPUPower(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    ;
    virtual QByteArray deserializeAndSetData(const QByteArray&) ;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::GPUPower& gpuPower,Origin origin);

public:

    static constexpr quint8  dataType = legion::messages::DataType::GPU_POWER;
};

}
