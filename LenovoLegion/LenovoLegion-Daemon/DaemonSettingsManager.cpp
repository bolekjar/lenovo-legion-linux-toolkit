// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Settings.h"
#include "DaemonSettingsManager.h"

#include "DataProviderManager.h"
#include "Modules/ModuleDataProviderAdapter.h"

#include <algorithm>

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

DaemonSettingsManager& DaemonSettingsManager::getInstance()
{
    static DaemonSettingsManager instance;
    return instance;
}

DaemonSettingsManager::DaemonSettingsManager() : m_daemonSettings{}
{
}

void DaemonSettingsManager::loadAllSettings(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadAllSettings - start");
    
    // Always load daemon settings first
    loadDaemonSettings();
    
    // Check if hardware settings should be applied on start
    if (!m_daemonSettings.apply_settings_on_start()) {
        LOG_D("DaemonSettingsManager::loadAllSettings - apply_settings_on_start is disabled, skipping hardware settings load");
        return;
    }
    
    LOG_T("DaemonSettingsManager::loadAllSettings - applying saved hardware settings");

    applyModuleSettings(dataProviderManager,{});

    LOG_T("DaemonSettingsManager::loadAllSettings - complete");
}

void DaemonSettingsManager::applyModuleSettings(DataProviderManager* dataProviderManager,const QString& prefix)
{
    // Load power profile first to check if CUSTOM mode is saved
    legion::messages::PowerProfile savedProfile;
    SettingsLoaderPowerProfiles(prefix).loadPowerProfile(savedProfile);
    bool isCustomProfile = savedProfile.current_value() &&
                          savedProfile.current_value() == legion::messages::PowerProfile_Profiles_POWER_PROFILE_CUSTOM;
    
    if (isCustomProfile) {
        LOG_D("DaemonSettingsManager::loadAllSettings - CUSTOM power profile detected, will load custom settings");
    } else {
        LOG_D("DaemonSettingsManager::loadAllSettings - Non-CUSTOM power profile, will skip custom-only settings");
    }
    
    /*
     * Daemon modules in the order of their settings policy (PowerProfile first),
     * custom only modules (FanCurve, CPUPower, GPUPower) only when CUSTOM power profile is saved
     */
    for (auto* module : persistentModules(dataProviderManager)) {

        if (module->settingsPolicy().customOnly && !isCustomProfile) {
            LOG_D(QString("DaemonSettingsManager::loadAllSettings - skipping ").append(module->info().name).append(" (not CUSTOM profile)"));
            continue;
        }

        LOG_T(QString("DaemonSettingsManager::loadAllSettings - ").append(module->info().name));
        module->loadSettings(prefix);
    }
}

void DaemonSettingsManager::saveAllSettings(DataProviderManager* dataProviderManager)
{
    LOG_D("DaemonSettingsManager::saveAllSettings - start");

    saveDaemonSettings();

    storeModuleSettings(dataProviderManager,{});

    LOG_D("DaemonSettingsManager::saveAllSettings - complete");
}

void DaemonSettingsManager::storeModuleSettings(DataProviderManager* dataProviderManager,const QString& prefix)
{
    for (auto* module : persistentModules(dataProviderManager)) {
        LOG_T(QString("DaemonSettingsManager::saveAllSettings - ").append(module->info().name));
        module->saveSettings(prefix);
    }
}

std::vector<std::pair<QString,QString>> DaemonSettingsManager::profiles()
{
    SettingsProfiles                        settings;
    std::vector<std::pair<QString,QString>> profiles;

    for (const auto& name : settings.names()) {
        profiles.emplace_back(name,settings.description(name));
    }

    return profiles;
}

void DaemonSettingsManager::saveProfile(DataProviderManager* dataProviderManager,const QString& name,const QString& description)
{
    checkProfileName(name);

    LOG_D(QString("DaemonSettingsManager::saveProfile - ").append(name));

    /*
     * Overwrite: values of the old profile which are not settable now must not stay in the profile
     */
    {
        SettingsProfiles settings;

        settings.remove(name);
        settings.setDescription(name,description);
    }

    storeModuleSettings(dataProviderManager,SettingsProfiles::prefix(name));
}

void DaemonSettingsManager::loadProfile(DataProviderManager* dataProviderManager,const QString& name)
{
    checkProfileName(name);

    if (!SettingsProfiles().exists(name)) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROFILE_ERROR,QString("Profile ").append(name).append(" does not exist !").toStdString().c_str());
    }

    LOG_D(QString("DaemonSettingsManager::loadProfile - ").append(name));

    applyModuleSettings(dataProviderManager,SettingsProfiles::prefix(name));
}

void DaemonSettingsManager::deleteProfile(const QString& name)
{
    checkProfileName(name);

    LOG_D(QString("DaemonSettingsManager::deleteProfile - ").append(name));

    SettingsProfiles().remove(name);
}

void DaemonSettingsManager::checkProfileName(const QString& name)
{
    /*
     * Name is a settings group, slash and backslash would create nested groups
     */
    if (name.trimmed().isEmpty() || name != name.trimmed() || name.size() > MAX_PROFILE_NAME_LENGTH || name.contains('/') || name.contains('\\')) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROFILE_ERROR,QString("Invalid profile name \"").append(name).append("\" !").toStdString().c_str());
    }
}

std::vector<ModuleDataProviderAdapter*> DaemonSettingsManager::persistentModules(DataProviderManager* dataProviderManager)
{
    std::vector<ModuleDataProviderAdapter*> modules;

    dataProviderManager->forEachDataProviderDo([&modules](DataProvider& provider) {
        auto* module = dynamic_cast<ModuleDataProviderAdapter*>(&provider);

        if (module != nullptr && module->settingsPolicy().persistent) {
            modules.push_back(module);
        }
    });

    std::stable_sort(modules.begin(),modules.end(),[](const ModuleDataProviderAdapter* a,const ModuleDataProviderAdapter* b) {
        return a->settingsPolicy().order < b->settingsPolicy().order;
    });

    return modules;
}

void DaemonSettingsManager::saveAllSettingsOnExit(DataProviderManager *dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveAllSettingsOnExit - start");

    if (!shouldSaveOnExit()) {
        LOG_D("DaemonSettingsManager::saveAllSettingsOnExit - save disabled by user settings");
        return;
    }

    saveAllSettings(dataProviderManager);

    LOG_T("DaemonSettingsManager::saveAllSettingsOnExit - complete");
}

void DaemonSettingsManager::loadDaemonSettings()
{
    LOG_T("DaemonSettingsManager::loadDaemonSettings");
    SettingsLoaderDaemonSettings loader;
    loader.loadDaemonSettings(m_daemonSettings);
    
    // Apply debug and trace logging levels immediately after loading
    bj::framework::Logger::SEVERITY_BITSET severity;
    
    // Always include base levels
    severity = bj::framework::Logger::SEVERITY_BITSET()
             .set(bj::framework::Logger::ERROR)
             .set(bj::framework::Logger::WARNING)
             .set(bj::framework::Logger::INFO);
    
    // Add debug if enabled
    if(m_daemonSettings.debug_logging())
    {
        LOG_D("Debug logging enabled from settings");
        severity.set(bj::framework::Logger::DEBUG);
    }
    
    // Add trace if enabled
    if(m_daemonSettings.trace_logging())
    {
        LOG_D("Trace logging enabled from settings");
        severity.set(bj::framework::Logger::TRACE);
    }
    
    LoggerHolder::getInstance().setSeverity(severity);
}

void DaemonSettingsManager::saveDaemonSettings()
{
    LOG_D("DaemonSettingsManager::saveDaemonSettings");
    SettingsSaverDaemonSettings saver;
    saver.saveDaemonSettings(m_daemonSettings);
}

const legion::messages::DaemonSettings& DaemonSettingsManager::getDaemonSettings() const
{
    return m_daemonSettings;
}

void DaemonSettingsManager::setDaemonSettings(const legion::messages::DaemonSettings& settings)
{
    m_daemonSettings = settings;
}

bool DaemonSettingsManager::shouldSaveOnExit() const
{
    return m_daemonSettings.save_settings_on_exit();
}

}
