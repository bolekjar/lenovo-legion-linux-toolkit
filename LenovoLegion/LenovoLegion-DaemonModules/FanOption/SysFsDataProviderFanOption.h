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
#include <FanControl.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderFanOption : public SysFsDataProvider
{
public:

    SysFsDataProviderFanOption(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                   ;
    virtual QByteArray deserializeAndSetData(const QByteArray&);

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::FanOption& fanOption,Origin origin) const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::FAN_OPTION;
};

}
