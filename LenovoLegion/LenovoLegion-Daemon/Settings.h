// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QSettings>

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

class SettingsLoaderPowerProfiles: protected Settings
{
public:
    explicit SettingsLoaderPowerProfiles();
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

//CPU PState Settings

}
