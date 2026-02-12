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

namespace LenovoLegionDaemon {

class DataProviderManager;

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

    // Load individual setting types
    void loadDaemonSettings();
    void loadPowerProfile(DataProviderManager* dataProviderManager);
    void loadCPUControlData(DataProviderManager* dataProviderManager);
    void loadCPUFrequency(DataProviderManager* dataProviderManager);
    void loadFanCurve(DataProviderManager* dataProviderManager);
    void loadFanOption(DataProviderManager* dataProviderManager);
    void loadCPUSMT(DataProviderManager* dataProviderManager);
    void loadCPUPower(DataProviderManager* dataProviderManager);
    void loadGPUPower(DataProviderManager* dataProviderManager);
    void loadNvidiaNvml(DataProviderManager* dataProviderManager);
    void loadIntelMSR(DataProviderManager* dataProviderManager);
    void loadOther(DataProviderManager* dataProviderManager);

    // Save individual setting types
    void saveDaemonSettings();
    void savePowerProfile(DataProviderManager* dataProviderManager);
    void saveCPUControlData(DataProviderManager* dataProviderManager);
    void saveCPUFrequency(DataProviderManager* dataProviderManager);
    void saveFanCurve(DataProviderManager* dataProviderManager);
    void saveFanOption(DataProviderManager* dataProviderManager);
    void saveCPUSMT(DataProviderManager* dataProviderManager);
    void saveCPUPower(DataProviderManager* dataProviderManager);
    void saveGPUPower(DataProviderManager* dataProviderManager);
    void saveNvidiaNvml(DataProviderManager* dataProviderManager);
    void saveIntelMSR(DataProviderManager* dataProviderManager);
    void saveOther(DataProviderManager* dataProviderManager);

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
