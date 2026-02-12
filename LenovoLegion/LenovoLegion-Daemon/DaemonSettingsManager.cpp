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
#include "SysFsDataProviderPowerProfile.h"
#include "SysFsDataProviderCPUOptions.h"
#include "SysFsDataProviderCPUFrequency.h"
#include "SysFsDataProviderFanCurve.h"
#include "SysFsDataProviderFanOption.h"
#include "SysFsDataProviderCPUSMT.h"
#include "SysFsDataProviderCPUPower.h"
#include "SysFsDataProviderGPUPower.h"
#include "DataProviderNvidiaNvml.h"
#include "SysFsDataProviderIntelMSR.h"
#include "SysFsDataProviderOther.h"

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
    
    // Load power profile first to check if CUSTOM mode is saved
    legion::messages::PowerProfile savedProfile;
    SettingsLoaderPowerProfiles().loadPowerProfile(savedProfile);
    bool isCustomProfile = savedProfile.current_value() &&
                          savedProfile.current_value() == legion::messages::PowerProfile_Profiles_POWER_PROFILE_CUSTOM;
    
    if (isCustomProfile) {
        LOG_D("DaemonSettingsManager::loadAllSettings - CUSTOM power profile detected, will load custom settings");
    } else {
        LOG_D("DaemonSettingsManager::loadAllSettings - Non-CUSTOM power profile, will skip custom-only settings");
    }
    
    // Always load power profile
    loadPowerProfile(dataProviderManager);
    loadCPUControlData(dataProviderManager);
    loadCPUFrequency(dataProviderManager);
    
    // Only load custom settings if CUSTOM power profile is saved
    if (isCustomProfile) {
        loadFanCurve(dataProviderManager);
        loadCPUPower(dataProviderManager);
        loadGPUPower(dataProviderManager);
    } else {
        LOG_D("DaemonSettingsManager::loadAllSettings - skipping FanCurve, CPUPower, GPUPower (not CUSTOM profile)");
    }
    
    loadFanOption(dataProviderManager);
    loadCPUSMT(dataProviderManager);
    loadNvidiaNvml(dataProviderManager);
    loadIntelMSR(dataProviderManager);
    loadOther(dataProviderManager);
    LOG_T("DaemonSettingsManager::loadAllSettings - complete");
}

void DaemonSettingsManager::saveAllSettings(DataProviderManager* dataProviderManager)
{
    LOG_D("DaemonSettingsManager::saveAllSettings - start");

    saveDaemonSettings();
    savePowerProfile(dataProviderManager);
    saveCPUControlData(dataProviderManager);
    saveCPUFrequency(dataProviderManager);
    saveFanCurve(dataProviderManager);
    saveFanOption(dataProviderManager);
    saveCPUSMT(dataProviderManager);
    saveCPUPower(dataProviderManager);
    saveGPUPower(dataProviderManager);
    saveNvidiaNvml(dataProviderManager);
    saveIntelMSR(dataProviderManager);
    saveOther(dataProviderManager);
    LOG_D("DaemonSettingsManager::saveAllSettings - complete");
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

void DaemonSettingsManager::loadPowerProfile(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadPowerProfile");
    try {
        legion::messages::PowerProfile profile;
        SettingsLoaderPowerProfiles().loadPowerProfile(profile);
        // Skip if no fields are set
        if(profile.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadPowerProfile - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(profile.ByteSizeLong());
        if(profile.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderPowerProfile::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadPowerProfile - failed");
    }
}

void DaemonSettingsManager::savePowerProfile(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::savePowerProfile");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderPowerProfile::dataType).serializeAndGetData();
        legion::messages::PowerProfile profile;
        if(profile.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverPowerProfiles().saverPowerProfile(profile);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::savePowerProfile - failed");
    }
}

void DaemonSettingsManager::loadCPUControlData(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadCPUControlData");
    try {
        legion::messages::CPUOptions opts;
        SettingsLoaderCPUControlData().loadPowerProfile(opts);
        // Skip if no fields are set
        if(opts.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadCPUControlData - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(opts.ByteSizeLong());
        if(opts.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderCPUOptions::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadCPUControlData - failed");
    }
}

void DaemonSettingsManager::saveCPUControlData(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveCPUControlData");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderCPUOptions::dataType).serializeAndGetData();
        legion::messages::CPUOptions opts;
        if(opts.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverCPUControlData().saverPowerProfile(opts);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveCPUControlData - failed");
    }
}

void DaemonSettingsManager::loadCPUFrequency(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadCPUFrequency");
    try {
        legion::messages::CPUFrequency freq;
        SettingsLoaderCPUFrequency().loadCPUFrequency(freq);
        // Skip if no fields are set
        if(freq.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadCPUFrequency - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(freq.ByteSizeLong());
        if(freq.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderCPUFrequency::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadCPUFrequency - failed");
    }
}

void DaemonSettingsManager::saveCPUFrequency(DataProviderManager* dataProviderManager)
{
    LOG_D("DaemonSettingsManager::saveCPUFrequency");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderCPUFrequency::dataType).serializeAndGetData();
        legion::messages::CPUFrequency freq;
        if(freq.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverCPUFrequency().saveCPUFrequency(freq);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveCPUFrequency - failed");
    }
}

void DaemonSettingsManager::loadFanCurve(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadFanCurve");
    try {
        legion::messages::FanCurve curve;
        SettingsLoaderFanCurve().loadFanCurve(curve);
        // Skip if no fields are set
        if(curve.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadFanCurve - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(curve.ByteSizeLong());
        if(curve.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderFanCurve::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadFanCurve - failed");
    }
}

void DaemonSettingsManager::saveFanCurve(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveFanCurve");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderFanCurve::dataType).serializeAndGetData();
        legion::messages::FanCurve curve;
        if(curve.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverFanCurve().saveFanCurve(curve);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveFanCurve - failed");
    }
}

void DaemonSettingsManager::loadFanOption(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadFanOption");
    try {
        legion::messages::FanOption opt;
        SettingsLoaderFanOption().loadFanOption(opt);
        // Skip if no fields are set
        if(opt.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadFanOption - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(opt.ByteSizeLong());
        if(opt.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderFanOption::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadFanOption - failed");
    }
}

void DaemonSettingsManager::saveFanOption(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveFanOption");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderFanOption::dataType).serializeAndGetData();
        legion::messages::FanOption opt;
        if(opt.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverFanOption().saveFanOption(opt);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveFanOption - failed");
    }
}

void DaemonSettingsManager::loadCPUSMT(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadCPUSMT");
    try {
        legion::messages::CPUSMT smt;
        SettingsLoaderCPUSMT().loadCPUSMT(smt);
        // Skip if no fields are set
        if(smt.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadCPUSMT - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(smt.ByteSizeLong());
        if(smt.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderCPUSMT::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadCPUSMT - failed");
    }
}

void DaemonSettingsManager::saveCPUSMT(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveCPUSMT");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderCPUSMT::dataType).serializeAndGetData();
        legion::messages::CPUSMT smt;
        if(smt.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverCPUSMT().saveCPUSMT(smt);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveCPUSMT - failed");
    }
}

void DaemonSettingsManager::loadCPUPower(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadCPUPower");
    try {
        legion::messages::CPUPower power;
        SettingsLoaderCPUPower().loadCPUPower(power);
        // Skip if no fields are set
        if(power.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadCPUPower - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(power.ByteSizeLong());
        if(power.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderCPUPower::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadCPUPower - failed");
    }
}

void DaemonSettingsManager::saveCPUPower(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveCPUPower");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderCPUPower::dataType).serializeAndGetData();
        legion::messages::CPUPower power;
        if(power.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverCPUPower().saveCPUPower(power);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveCPUPower - failed");
    }
}

void DaemonSettingsManager::loadGPUPower(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadGPUPower");
    try {
        legion::messages::GPUPower power;
        SettingsLoaderGPUPower().loadGPUPower(power);
        // Skip if no fields are set
        if(power.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadGPUPower - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(power.ByteSizeLong());
        if(power.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderGPUPower::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadGPUPower - failed");
    }
}

void DaemonSettingsManager::saveGPUPower(DataProviderManager* dataProviderManager)
{
    LOG_D("DaemonSettingsManager::saveGPUPower");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderGPUPower::dataType).serializeAndGetData();
        legion::messages::GPUPower power;
        if(power.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverGPUPower().saveGPUPower(power);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveGPUPower - failed");
    }
}

void DaemonSettingsManager::loadNvidiaNvml(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadNvidiaNvml");
    try {
        legion::messages::NvidiaNvml nvml;
        SettingsLoaderNvidiaNvml().loadNvidiaNvml(nvml);
        // Skip if no fields are set
        if(nvml.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadNvidiaNvml - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(nvml.ByteSizeLong());
        if(nvml.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(DataProviderNvidiaNvml::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadNvidiaNvml - failed");
    }
}

void DaemonSettingsManager::saveNvidiaNvml(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveNvidiaNvml");
    try {
        auto data = dataProviderManager->getDataProvider(DataProviderNvidiaNvml::dataType).serializeAndGetData();
        legion::messages::NvidiaNvml nvml;
        if(nvml.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverNvidiaNvml().saveNvidiaNvml(nvml);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveNvidiaNvml - failed");
    }
}

void DaemonSettingsManager::loadIntelMSR(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadIntelMSR");
    try {
        legion::messages::CpuIntelMSR intelMSR;
        SettingsLoaderIntelMSR().loadIntelMSR(intelMSR);
        // Skip if no fields are set
        if(intelMSR.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadIntelMSR - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(intelMSR.ByteSizeLong());
        if(intelMSR.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderIntelMSR::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadIntelMSR - failed");
    }
}

void DaemonSettingsManager::saveIntelMSR(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::saveIntelMSR");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderIntelMSR::dataType).serializeAndGetData();
        legion::messages::CpuIntelMSR intelMSR;
        if(intelMSR.ParseFromArray(data.data(), data.size()))
        {
            intelMSR.mutable_analogio()->set_offset(((intelMSR.analogio().offset() > 0 ? intelMSR.analogio().offset() + 999 : intelMSR.analogio().offset() - 999 ) / 1000) * 1000);
            intelMSR.mutable_cache()->set_offset(((intelMSR.cache().offset() > 0 ? intelMSR.cache().offset() + 999 : intelMSR.cache().offset() - 999 ) / 1000) * 1000);
            intelMSR.mutable_cpu()->set_offset(((intelMSR.cpu().offset() > 0 ? intelMSR.cpu().offset() + 999 : intelMSR.cpu().offset() - 999) / 1000) * 1000);
            intelMSR.mutable_gpu()->set_offset(((intelMSR.gpu().offset() > 0 ? intelMSR.gpu().offset() + 999 : intelMSR.gpu().offset() - 999 ) / 1000) * 1000);
            intelMSR.mutable_uncore()->set_offset(((intelMSR.uncore().offset() > 0 ? intelMSR.uncore().offset() + 999 : intelMSR.uncore().offset() - 999 ) / 1000) * 1000);

            SettingsSaverIntelMSR().saveIntelMSR(intelMSR);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveIntelMSR - failed");
    }
}

void DaemonSettingsManager::loadOther(DataProviderManager* dataProviderManager)
{
    LOG_T("DaemonSettingsManager::loadOther");
    try {
        legion::messages::OtherSettings otherSettings;
        SettingsLoaderOther().loadOther(otherSettings);
        // Skip if no fields are set
        if(otherSettings.ByteSizeLong() == 0) {
            LOG_D("DaemonSettingsManager::loadOther - no saved data, skipping");
            return;
        }
        QByteArray data;
        data.resize(otherSettings.ByteSizeLong());
        if(otherSettings.SerializeToArray(data.data(),data.size()))
        {
            dataProviderManager->getDataProvider(SysFsDataProviderOther::dataType).deserializeAndSetData(data);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::loadOther - failed");
    }
}

void DaemonSettingsManager::saveOther(DataProviderManager* dataProviderManager)
{
    LOG_D("DaemonSettingsManager::saveOther");
    try {
        auto data = dataProviderManager->getDataProvider(SysFsDataProviderOther::dataType).serializeAndGetData();
        legion::messages::OtherSettings otherSettings;
        if(otherSettings.ParseFromArray(data.data(), data.size()))
        {
            SettingsSaverOther().saveOther(otherSettings);
        }
    } catch(...) {
        LOG_W("DaemonSettingsManager::saveOther - failed");
    }
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
