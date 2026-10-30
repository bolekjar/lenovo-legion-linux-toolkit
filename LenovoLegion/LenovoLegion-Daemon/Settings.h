// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QSettings>
#include <QStringList>

#include "../LenovoLegion-PrepareBuild/PowerProfile.pb.h"
#include "../LenovoLegion-PrepareBuild/DaemonSettings.pb.h"

namespace LenovoLegionDaemon {

class Settings {

protected:

public:

    explicit Settings(const QString& group);
    virtual ~Settings();

    Settings(Settings&)  = delete;
    Settings(Settings&&) = delete;

    Settings& operator=(Settings&)  = delete;
    Settings& operator=(Settings&&) = delete;

protected:

    QSettings m_settings;
};

/*
 * Saved power profile of the module PowerProfile, prefix selects the saved settings (empty) or a profile (SettingsProfiles::prefix)
 */
class SettingsLoaderPowerProfiles: protected Settings
{
public:
    explicit SettingsLoaderPowerProfiles(const QString& prefix = {});
    SettingsLoaderPowerProfiles& loadPowerProfile(legion::messages::PowerProfile& profile);
};

// Daemon Settings
class SettingsLoaderDaemonSettings: protected Settings
{
public:
    explicit SettingsLoaderDaemonSettings();
    SettingsLoaderDaemonSettings& loadDaemonSettings(legion::messages::DaemonSettings &daemonSettings);
};

class SettingsSaverDaemonSettings: protected Settings
{
public:
    explicit SettingsSaverDaemonSettings();
    SettingsSaverDaemonSettings& saveDaemonSettings(const legion::messages::DaemonSettings &daemonSettings);
};

/*
 * Saved profiles: group Profiles/<name>/ with the description and the settings groups of the daemon modules
 */
class SettingsProfiles: protected Settings
{
public:
    explicit SettingsProfiles();

    /*
     * Prefix of the settings groups of the profile
     */
    static QString prefix(const QString& name);

    QStringList names();
    bool        exists(const QString& name);
    QString     description(const QString& name);
    void        setDescription(const QString& name,const QString& description);
    void        remove(const QString& name);
};

}
