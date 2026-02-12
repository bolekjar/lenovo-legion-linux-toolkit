// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):\n *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProvider.h"

namespace LenovoLegionDaemon {

class DataProviderManager;

class DataProviderDaemonSettings : public DataProvider
{
    Q_OBJECT

public:
    explicit DataProviderDaemonSettings(DataProviderManager* dataProviderManager);
    ~DataProviderDaemonSettings() override = default;

    QByteArray serializeAndGetData() const override;
    QByteArray deserializeAndSetData(const QByteArray& data) override;

    void init() override;
    void clean() override;

public:
    static constexpr quint8 dataType = 16;

private:
    DataProviderManager* m_dataProviderManager;
};

}
