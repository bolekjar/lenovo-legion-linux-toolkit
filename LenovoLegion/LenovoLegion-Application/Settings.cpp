// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Settings.h>

#include <Core/Application.h>
#include <Core/LoggerHolder.h>


#include <QDir>

namespace  LenovoLegionGui {

Settings::Settings(const QString &group):
    m_settings(std::filesystem::path(QCoreApplication::applicationDirPath().toStdString()).append(bj::framework::Application::data_dir).append(CONFIG_NAME.toStdString()).string().c_str(), QSettings::IniFormat)
{
    m_settings.setValue("LenovoLegion","LenovoLegionGui");
    m_settings.beginGroup(group);
}

Settings::~Settings()
{
    m_settings.endGroup();
}

const QString Settings::CONFIG_NAME = "LenovoLegionGui.ini";


ApplicationSettings::ApplicationSettings(QObject* parent)
    : QObject(parent)
    , Settings("Application")
{}

ApplicationSettings* ApplicationSettings::instance()
{
    static ApplicationSettings s_instance;
    return &s_instance;
}

ApplicationSettings& ApplicationSettings::loadStartMinimized(bool& value)
{
    value = m_settings.value("StartMinimized", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadMinimizeToTray(bool& value)
{
    value = m_settings.value("MinimizeToTray", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadAppDebugLogging(bool& value)
{
    value = m_settings.value("AppDebugLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadAppTraceLogging(bool& value)
{
    value = m_settings.value("AppTraceLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadSaveSettingsOnDaemonExit(bool& value)
{
    value = m_settings.value("SaveSettingsOnDaemonExit", true).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadDaemonDebugLogging(bool& value)
{
    value = m_settings.value("DaemonDebugLogging", false).toBool();
    return *this;
}

ApplicationSettings& ApplicationSettings::loadStylesheetTheme(ThemeType& value)
{
    value = static_cast<ThemeType>(m_settings.value("StylesheetTheme", static_cast<int>(ThemeType::NoTheme)).toInt());
    return *this;
}

ApplicationSettings& ApplicationSettings::saveStartMinimized(bool value)
{
    m_settings.setValue("StartMinimized", value);
    emit settingChanged(SettingType::StartMinimized, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveMinimizeToTray(bool value)
{
    m_settings.setValue("MinimizeToTray", value);
    emit settingChanged(SettingType::MinimizeToTray, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveAppDebugLogging(bool value)
{
    m_settings.setValue("AppDebugLogging", value);
    emit settingChanged(SettingType::AppDebugLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveAppTraceLogging(bool value)
{
    m_settings.setValue("AppTraceLogging", value);
    emit settingChanged(SettingType::AppTraceLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveSaveSettingsOnDaemonExit(bool value)
{
    m_settings.setValue("SaveSettingsOnDaemonExit", value);
    emit settingChanged(SettingType::SaveSettingsOnDaemonExit, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveDaemonDebugLogging(bool value)
{
    m_settings.setValue("DaemonDebugLogging", value);
    emit settingChanged(SettingType::DaemonDebugLogging, value);
    return *this;
}

ApplicationSettings& ApplicationSettings::saveStylesheetTheme(ThemeType value)
{
    m_settings.setValue("StylesheetTheme", static_cast<int>(value));
    emit themeChanged(value);
    return *this;
}

void ApplicationSettings::resetToDefaults()
{
    saveStartMinimized(false)
        .saveMinimizeToTray(false)
        .saveAppDebugLogging(false)
        .saveAppTraceLogging(false)
        .saveSaveSettingsOnDaemonExit(true)
        .saveDaemonDebugLogging(false)
        .saveStylesheetTheme(ThemeType::NoTheme);
}

}
