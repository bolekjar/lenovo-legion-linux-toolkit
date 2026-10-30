// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../DataProvider.h"

#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"

namespace LenovoLegionDaemon {

class DataProviderManager;

/*
 * Saved profiles of the settings (DaemonSettingsManager), GET returns the list, SET executes ProfileCommand.
 * Failed command (invalid name, unknown profile) throws, the client gets ERROR_RESPONSE.
 */
class DataProviderProfiles : public DataProvider
{
    Q_OBJECT

public:
    explicit DataProviderProfiles(DataProviderManager* dataProviderManager);
    ~DataProviderProfiles() override = default;

    QByteArray serializeAndGetData() override;
    QByteArray deserializeAndSetData(const QByteArray& data) override;

    void init() override;
    void clean() override;

public:
    static constexpr quint8 dataType = legion::messages::DataType::PROFILES;

private:
    DataProviderManager* m_dataProviderManager;
};

}
