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
#include <PowerProfile.pb.h>

#include <vector>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderPowerProfile : public SysFsDataProvider
{
public:

    SysFsDataProviderPowerProfile(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    ;
    virtual QByteArray deserializeAndSetData(const QByteArray&) ;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    std::vector<legion::messages::PowerProfile::Profiles> supportedProfiles() const;

    void settable(legion::messages::PowerProfile& profile,Origin origin) const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::POWER_PROFILE;
};

}
