// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QObject>

#include <functional>

namespace LenovoLegionGui {

/*
 * Settings of the application (ApplicationSettings, LenovoLegionGui.ini) for QML pages (context property "appSettings",
 * PreferencesPage): every change is saved at once and announced by ApplicationSettings (theme, logging, tray).
 * reconnectDaemon() connects to the daemon again (set by MainWindow, setReconnect).
 */
class AppSettings : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool startMinimized READ startMinimized WRITE setStartMinimized NOTIFY changed)
    Q_PROPERTY(bool minimizeToTray READ minimizeToTray WRITE setMinimizeToTray NOTIFY changed)
    Q_PROPERTY(bool debugLogging   READ debugLogging   WRITE setDebugLogging   NOTIFY changed)
    Q_PROPERTY(bool traceLogging   READ traceLogging   WRITE setTraceLogging   NOTIFY changed)

    // ApplicationSettings::ThemeType: 0 no theme, 1 white, 2 gray, 3 dark
    Q_PROPERTY(int  theme          READ theme          WRITE setTheme          NOTIFY changed)

public:

    explicit AppSettings(QObject* parent = nullptr);

    bool startMinimized() const;
    bool minimizeToTray() const;
    bool debugLogging() const;
    bool traceLogging() const;
    int  theme() const;

    void setStartMinimized(bool value);
    void setMinimizeToTray(bool value);
    void setDebugLogging(bool value);
    void setTraceLogging(bool value);
    void setTheme(int value);

    Q_INVOKABLE void resetToDefaults();
    Q_INVOKABLE void reconnectDaemon();

    /*
     * Reconnection to the daemon (DataProviderManager of MainWindow)
     */
    static void setReconnect(const std::function<void ()>& reconnect);

signals:

    void changed();

private:

    static std::function<void ()> s_reconnect;
};

}
