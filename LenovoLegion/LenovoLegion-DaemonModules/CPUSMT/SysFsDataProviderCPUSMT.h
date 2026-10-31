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
#include <CPUOptions.pb.h>

namespace LenovoLegionDaemon::Kit {

class SysFsDataProviderCPUSMT : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUSMT(SysFsDriverManager* sysFsDriverManager);


    virtual QByteArray serializeAndGetData()                    ;
    virtual QByteArray deserializeAndSetData(const QByteArray&) ;

    QByteArray settableData(const QByteArray& data,Origin origin) override;

private:

    void settable(legion::messages::CPUSMT& smt,Origin origin) const;

    /*
     * Control values which can be set when the current control is the given one (empty = not switchable)
     */
    static QStringList settableControls(const QString& current);

public:

    static constexpr quint8  dataType = legion::messages::DataType::CPU_SMT;
};

}
