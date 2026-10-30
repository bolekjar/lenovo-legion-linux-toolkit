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

#include <QString>

#include <utility>
#include <vector>

namespace LenovoLegionDaemon {

class DataProviderManager;
class ModuleDataProviderAdapter;

class DaemonSettingsManager
{
public:
    DEFINE_EXCEPTION(DaemonSettingsManager);

    enum ERROR_CODES : int {
        LOAD_ERROR      = -1,
        SAVE_ERROR      = -2,
        PROFILE_ERROR   = -3
    };

    static constexpr qsizetype MAX_PROFILE_NAME_LENGTH = 64;

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

    /*
     * Saved profiles (name, description), a profile is the same as the saved settings of the modules in its own groups
     */
    std::vector<std::pair<QString,QString>> profiles();
    void saveProfile(DataProviderManager* dataProviderManager,const QString& name,const QString& description);
    void loadProfile(DataProviderManager* dataProviderManager,const QString& name);
    void deleteProfile(const QString& name);

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

    /*
     * Settings of the modules in the order of their policy, prefix selects the saved settings (empty) or a profile
     */
    void applyModuleSettings(DataProviderManager* dataProviderManager,const QString& prefix);
    void storeModuleSettings(DataProviderManager* dataProviderManager,const QString& prefix);

    static void checkProfileName(const QString& name);
    ~DaemonSettingsManager() = default;

    legion::messages::DaemonSettings m_daemonSettings;
};

}
