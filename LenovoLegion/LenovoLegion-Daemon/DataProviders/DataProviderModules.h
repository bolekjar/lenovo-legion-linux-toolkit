// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../DataProvider.h"
#include "../Modules/ModuleLoader.h"

#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"

#include <vector>

namespace LenovoLegionDaemon {

class DataProviderManager;

/*
 * List of the data providers of the daemon (DataType MODULES, read only): the daemon modules with their state, the
 * providers of the framework and the module files which were not loaded. The GUI does not request a data type without
 * a running provider and shows why it is not available.
 */
class DataProviderModules : public DataProvider
{
    Q_OBJECT

public:
    explicit DataProviderModules(DataProviderManager* dataProviderManager);
    ~DataProviderModules() override = default;

    QByteArray serializeAndGetData() override;

    /*
     * Modules which were not loaded (ModuleLoader::loadModules)
     */
    void setNotLoaded(const std::vector<ModuleLoader::NotLoaded>& notLoaded);

public:
    static constexpr quint8 dataType = legion::messages::DataType::MODULES;

private:
    DataProviderManager*                 m_dataProviderManager;
    std::vector<ModuleLoader::NotLoaded> m_notLoaded;
};

}
