// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

namespace LenovoLegionDaemon {

class SysFsDataProviderCPUSMT : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUSMT(SysFsDriverManager* sysFsDriverManager,QObject* parent);


    virtual QByteArray serializeAndGetData()                    const;
    virtual QByteArray deserializeAndSetData(const QByteArray&)      ;

public:

    static constexpr quint8  dataType = 10;
};

}
