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

class SysFsDataProviderFanCurve : public SysFsDataProvider
{
public:

    SysFsDataProviderFanCurve(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                         override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)      override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

    /*
     * Older firmware (the fan curve can not be read back, it does not keep it): the last written fan curve is
     * written again, nothing on the newer firmware
     */
    void writeLastAgain();

private:

    static void settable(legion::messages::FanCurve& fanCurve,Origin origin);

    /*
     * Last fan curve written (levels as text), the older firmware can not read it back
     */
    QStringList m_lastWritten;

public:

    static constexpr quint8  dataType = legion::messages::DataType::FAN_CURVE;
};

}
