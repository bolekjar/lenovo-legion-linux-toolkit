// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Settings.h>
#include <Core/Application.h>

namespace LenovoLegionDaemon {

Settings::Settings(const QString &group) :
    m_settings(QCoreApplication::applicationDirPath()
               .append(QDir::separator())
               .append(bj::framework::Application::data_dir)
               .append(QDir::separator())
               .append("/LenovoLegion-Daemon.ini"), QSettings::IniFormat)
{
    m_settings.setValue("LenovoLegion", "LenovoLegion-Daemon");
    m_settings.beginGroup(group);
}

Settings::~Settings()
{
    m_settings.endGroup();
}

SettingsLoaderPowerProfiles::SettingsLoaderPowerProfiles(const QString& prefix) :
    Settings(QString(prefix).append("PowerProfileData"))
{
}

SettingsLoaderPowerProfiles& SettingsLoaderPowerProfiles::loadPowerProfile(legion::messages::PowerProfile &profile)
{
    if (m_settings.contains("PowerProfile")) {
        profile.set_current_value(static_cast<legion::messages::PowerProfile::Profiles>(
            m_settings.value("PowerProfile").toUInt()));
    }

    if(m_settings.contains("custom_fnq_enabled")) {
        profile.mutable_custom_fnq_enabled()->set_current_value(m_settings.value("custom_fnq_enabled").toBool());
    }

    return *this;
}

// Daemon Settings
SettingsLoaderDaemonSettings::SettingsLoaderDaemonSettings() :
    Settings("DaemonSettings")
{
}

SettingsLoaderDaemonSettings& SettingsLoaderDaemonSettings::loadDaemonSettings(legion::messages::DaemonSettings &daemonSettings)
{
    daemonSettings.set_apply_settings_on_start(m_settings.value("apply_settings_on_start", true).toBool());
    daemonSettings.set_save_settings_on_exit(m_settings.value("save_settings_on_exit", true).toBool());
    daemonSettings.set_debug_logging(m_settings.value("debug_logging", false).toBool());
    daemonSettings.set_trace_logging(m_settings.value("trace_logging", false).toBool());
    // Note: save_now is not loaded - it's a command flag, not a persistent setting
    return *this;
}

SettingsSaverDaemonSettings::SettingsSaverDaemonSettings() :
    Settings("DaemonSettings")
{
}

SettingsSaverDaemonSettings& SettingsSaverDaemonSettings::saveDaemonSettings(const legion::messages::DaemonSettings &daemonSettings)
{
    m_settings.setValue("apply_settings_on_start", daemonSettings.apply_settings_on_start());
    m_settings.setValue("save_settings_on_exit", daemonSettings.save_settings_on_exit());
    m_settings.setValue("debug_logging", daemonSettings.debug_logging());
    m_settings.setValue("trace_logging", daemonSettings.trace_logging());
    // Note: save_now is not saved - it's a command flag, not a persistent setting
    return *this;
}

// Profiles
SettingsProfiles::SettingsProfiles() :
    Settings("Profiles")
{
}

QString SettingsProfiles::prefix(const QString &name)
{
    return QString("Profiles/").append(name).append('/');
}

QStringList SettingsProfiles::names()
{
    return m_settings.childGroups();
}

bool SettingsProfiles::exists(const QString &name)
{
    return names().contains(name);
}

QString SettingsProfiles::description(const QString &name)
{
    return m_settings.value(QString(name).append("/description")).toString();
}

void SettingsProfiles::setDescription(const QString &name, const QString &description)
{
    m_settings.setValue(QString(name).append("/description"),description);
}

void SettingsProfiles::remove(const QString &name)
{
    m_settings.remove(name);
}

}
