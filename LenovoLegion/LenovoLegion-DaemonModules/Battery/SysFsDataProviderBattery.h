// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include <Battery.pb.h>
#include <MessageRegistry.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderBattery : public SysFsDataProvider
{
public:

    SysFsDataProviderBattery(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                         override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)      override;

    /*
     * Power adapter now (POWER_ADAPTER_UNKNOWN without the data)
     */
    legion::messages::Battery::PowerAdapter powerAdapter();

public:

    static constexpr quint8  dataType = legion::messages::DataType::BATTERY;
};

}
