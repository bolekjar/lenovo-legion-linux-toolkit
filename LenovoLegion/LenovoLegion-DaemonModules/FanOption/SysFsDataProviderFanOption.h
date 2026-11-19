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


    virtual QByteArray serializeAndGetData()                   override;
    virtual QByteArray deserializeAndSetData(const QByteArray&) override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::FanOption& fanOption,Origin origin) const;

    /*
     * Files of the fan full speed: the other mode, on the older firmware the fan mode (Fan_Get/Set_FullSpeed)
     */
    struct FullSpeed {
        std::filesystem::path m_current_value;
        std::filesystem::path m_default_value;
        std::filesystem::path m_supported;
        QString               m_driverName;
    };

    FullSpeed fullSpeed() const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::FAN_OPTION;
};

}
