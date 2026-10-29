// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-PrepareBuild/DaemonSettings.pb.h"

#include <vector>

namespace LenovoLegionDaemon {

class DataProviderManager;
class ModuleDataProviderAdapter;

class DaemonSettingsManager
{
public:
    DEFINE_EXCEPTION(DaemonSettingsManager);

    enum ERROR_CODES : int {
        LOAD_ERROR = -1,
        SAVE_ERROR = -2
    };

    // Singleton instance access
    static DaemonSettingsManager& getInstance();

    // Delete copy constructor and assignment operator
    DaemonSettingsManager(const DaemonSettingsManager&) = delete;
    DaemonSettingsManager& operator=(const DaemonSettingsManager&) = delete;

    // Load all settings from storage
    void loadAllSettings(DataProviderManager* dataProviderManager);

    // Save all settings to storage
    void saveAllSettings(DataProviderManager* dataProviderManager);
    void saveAllSettingsOnExit(DataProviderManager* dataProviderManager);

    /*
     * Daemon modules with persistent settings, sorted by their settings order
     */
    static std::vector<ModuleDataProviderAdapter*> persistentModules(DataProviderManager* dataProviderManager);

    // Load individual setting types
    void loadDaemonSettings();

    // Save individual setting types
    void saveDaemonSettings();

    // Access daemon settings
    const legion::messages::DaemonSettings& getDaemonSettings() const;
    void setDaemonSettings(const legion::messages::DaemonSettings& settings);

    // Check if settings should be saved on exit
    bool shouldSaveOnExit() const;

private:
    DaemonSettingsManager();
    ~DaemonSettingsManager() = default;

    legion::messages::DaemonSettings m_daemonSettings;
};

}
