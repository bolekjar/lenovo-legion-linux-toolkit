// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QObject>
#include <QString>

#include <functional>

namespace LenovoLegionGui {

/*
 * State of the application for the window of the generic GUI (context property "appState", Shell.qml): connection to the
 * daemon, version, About (also from the tray menu) and Quit. Set by MainWindow.
 */
class AppState : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool    connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(QString version   READ version   CONSTANT)

public:

    AppState(const QString& version,const std::function<void ()>& reconnect,const std::function<void ()>& quit,QObject* parent = nullptr);

    bool    connected() const;
    QString version() const;

    void setConnected(bool connected);

    Q_INVOKABLE void reconnect();
    Q_INVOKABLE void quit();

signals:

    void connectedChanged();

    /*
     * The About dialog is opened (tray menu)
     */
    void aboutRequested();

private:

    bool                   m_connected = false;
    QString                m_version;
    std::function<void ()> m_reconnect;
    std::function<void ()> m_quit;
};

}
