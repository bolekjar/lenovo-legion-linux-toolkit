// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <SysFsDataProvider.h>

#include "../LenovoLegion-PrepareBuild/CpuPower.pb.h"

namespace LenovoLegionDaemon {

class SysFsDataProviderCPUPower : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUPower(SysFsDriverManager* sysFsDriverManager,QObject* parent);


    virtual QByteArray serializeAndGetData()                    const;
    virtual QByteArray deserializeAndSetData(const QByteArray&)      ;

private:

    void setValue(const std::filesystem::path& path,std::function<void (legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value)> setter,auto map) const;
    void setValuesSteps(const std::filesystem::path& path,std::function<void (legion::messages::CPUPower::Limit::Descriptor &descriptor,const QList<QString>& values)> setter,auto map) const;

public:

    static constexpr quint8  dataType = 4;
};

}
