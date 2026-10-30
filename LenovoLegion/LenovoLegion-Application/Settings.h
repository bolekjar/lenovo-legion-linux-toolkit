// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QSettings>
#include <QObject>
#include <QStringList>


namespace LenovoLegionGui {


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

public:

    static const QString CONFIG_NAME;
};


class ApplicationSettings : public QObject, protected Settings
{
    Q_OBJECT

public:
    enum class SettingType {
        StartMinimized,
        MinimizeToTray,
        AppDebugLogging,
        AppTraceLogging,
        SaveSettingsOnDaemonExit,
        DaemonDebugLogging,
        StylesheetTheme
    };
    Q_ENUM(SettingType)

    enum class ThemeType {
        NoTheme     = 0,
        WhiteTheme  = 1,
        GrayTheme   = 2,
        DarkTheme   = 3
    };
    Q_ENUM(ThemeType)

    explicit ApplicationSettings(QObject* parent = nullptr);

    // Singleton accessor
    static ApplicationSettings* instance();

    // Load settings
    ApplicationSettings& loadStartMinimized(bool& value);
    ApplicationSettings& loadMinimizeToTray(bool& value);
    ApplicationSettings& loadAppDebugLogging(bool& value);
    ApplicationSettings& loadAppTraceLogging(bool& value);
    ApplicationSettings& loadSaveSettingsOnDaemonExit(bool& value);
    ApplicationSettings& loadDaemonDebugLogging(bool& value);
    ApplicationSettings& loadStylesheetTheme(ThemeType& value);

    // Save settings
    ApplicationSettings& saveStartMinimized(bool value);
    ApplicationSettings& saveMinimizeToTray(bool value);
    ApplicationSettings& saveAppDebugLogging(bool value);
    ApplicationSettings& saveAppTraceLogging(bool value);
    ApplicationSettings& saveSaveSettingsOnDaemonExit(bool value);
    ApplicationSettings& saveDaemonDebugLogging(bool value);
    ApplicationSettings& saveStylesheetTheme(ThemeType value);

    // Reset to defaults
    void resetToDefaults();

signals:
    void settingChanged(LenovoLegionGui::ApplicationSettings::SettingType setting, bool value);
    void themeChanged(LenovoLegionGui::ApplicationSettings::ThemeType theme);
};


}
