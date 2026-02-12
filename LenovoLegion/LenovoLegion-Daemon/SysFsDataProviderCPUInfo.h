// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDataProvider.h"


namespace LenovoLegionDaemon {

class SysFsDataProviderCPUInfo : public SysFsDataProvider
{
public:

    SysFsDataProviderCPUInfo(SysFsDriverManager* sysFsDriverManager,QObject* parent);

    virtual QByteArray serializeAndGetData()                    const override;
    virtual QByteArray deserializeAndSetData(const QByteArray&)       override;

private:

    std::string getCpuName(int family, int model) const;
    void        parseModalias(const std::string& modalias, std::string& vendor,std::string& family, std::string& model) const;
    std::string getArchitecture(const std::string& modalias) const;
    std::string getCpuBrandString() const;
public:

    static constexpr quint8  dataType = 14;
};

}
