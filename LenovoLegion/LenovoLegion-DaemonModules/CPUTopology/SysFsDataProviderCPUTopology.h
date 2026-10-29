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

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderCPUTopology : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUTopology(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                         ;
    virtual QByteArray deserializeAndSetData(const QByteArray&)      ;

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_TOPOLOGY;
};

}
