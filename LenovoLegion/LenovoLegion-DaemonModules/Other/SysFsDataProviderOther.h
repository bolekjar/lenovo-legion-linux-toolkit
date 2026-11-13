// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>
#include <Drivers/SysFsDriverLegionVpc.h>

#include <MessageRegistry.pb.h>
#include <Other.pb.h>

#include <optional>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderOther : public SysFsDataProvider
{
public:

    SysFsDataProviderOther(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    override;
    virtual QByteArray deserializeAndSetData(const QByteArray&) override;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::OtherSettings& settings,Origin origin) const;

    /*
     * Features of the ACPI device VPC2004, empty without the driver (not on every model)
     */
    std::optional<SysFsDriverLegionVpc::Vpc> vpc() const;

public:

    static constexpr quint8  dataType = legion::messages::DataType::OTHER;
};

}
