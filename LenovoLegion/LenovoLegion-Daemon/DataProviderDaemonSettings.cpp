// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "DataProviderDaemonSettings.h"
#include "DaemonSettingsManager.h"
#include "DataProviderManager.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

DataProviderDaemonSettings::DataProviderDaemonSettings(DataProviderManager* dataProviderManager) : 
    DataProvider(dataProviderManager, dataType),
    m_dataProviderManager(dataProviderManager)
{}

QByteArray DataProviderDaemonSettings::serializeAndGetData() const
{
    legion::messages::DaemonSettings daemonSettings;
    QByteArray byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    // Get current daemon settings from singleton
    daemonSettings = DaemonSettingsManager::getInstance().getDaemonSettings();

    byteArray.resize(daemonSettings.ByteSizeLong());
    if(!daemonSettings.SerializeToArray(byteArray.data(), byteArray.size()))
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize of data message error !");
    }

    return byteArray;
}

QByteArray DataProviderDaemonSettings::deserializeAndSetData(const QByteArray& data)
{
    legion::messages::DaemonSettings newSettings;

    LOG_T(__PRETTY_FUNCTION__);

    if(!newSettings.ParseFromArray(data.data(), data.size()))
    {
        THROW_EXCEPTION(exception_T, DataProvider::ERROR_CODES::INVALID_DATA, "Parse of data message error !");
    }

    // Get old settings before updating to detect changes
    legion::messages::DaemonSettings oldSettings = DaemonSettingsManager::getInstance().getDaemonSettings();
    
    // Check if save_now command is set
    if(newSettings.has_save_now() && newSettings.save_now())
    {
        LOG_D("Save current configuration command received - saving all settings");
        DaemonSettingsManager::getInstance().saveAllSettings(m_dataProviderManager);
        
        // Reset the command flag so it doesn't persist
        newSettings.set_save_now(false);
    }
    
    // Check if debug_logging or trace_logging changed
    bool oldDebugLogging = oldSettings.debug_logging();
    bool newDebugLogging = newSettings.debug_logging();
    bool oldTraceLogging = oldSettings.trace_logging();
    bool newTraceLogging = newSettings.trace_logging();
    
    if(oldDebugLogging != newDebugLogging || oldTraceLogging != newTraceLogging)
    {
        LOG_D(QString("Logging settings changed - Debug: ") + QString::number(oldDebugLogging) + " -> " + QString::number(newDebugLogging) +
              ", Trace: " + QString::number(oldTraceLogging) + " -> " + QString::number(newTraceLogging));
        
        // Apply new logging level immediately
        bj::framework::Logger::SEVERITY_BITSET severity;
        
        // Always include base levels
        severity = bj::framework::Logger::SEVERITY_BITSET()
                 .set(bj::framework::Logger::ERROR)
                 .set(bj::framework::Logger::WARNING)
                 .set(bj::framework::Logger::INFO);
        
        // Add debug if enabled
        if(newDebugLogging)
        {
            severity.set(bj::framework::Logger::DEBUG);
        }
        
        // Add trace if enabled
        if(newTraceLogging)
        {
            severity.set(bj::framework::Logger::TRACE);
        }
        
        // Apply the new severity level
        LoggerHolder::getInstance().setSeverity(severity);
    }
    
    // Update daemon settings in singleton
    DaemonSettingsManager::getInstance().setDaemonSettings(newSettings);
    
    // Also save daemon settings to persistent storage
    DaemonSettingsManager::getInstance().saveDaemonSettings();

    return {};
}

void DataProviderDaemonSettings::init()
{}

void DataProviderDaemonSettings::clean()
{}

}
