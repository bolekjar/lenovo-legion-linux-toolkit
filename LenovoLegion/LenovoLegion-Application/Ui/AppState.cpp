// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "AppState.h"

namespace LenovoLegionGui {

AppState::AppState(const QString &version, const std::function<void ()> &reconnect, const std::function<void ()> &quit, QObject *parent) :
    QObject(parent),
    m_version(version),
    m_reconnect(reconnect),
    m_quit(quit)
{}

bool AppState::connected() const
{
    return m_connected;
}

QString AppState::version() const
{
    return m_version;
}

void AppState::setConnected(bool connected)
{
    if(connected != m_connected)
    {
        m_connected = connected;
        emit connectedChanged();
    }
}

void AppState::reconnect()
{
    if(m_reconnect)
    {
        m_reconnect();
    }
}

void AppState::quit()
{
    if(m_quit)
    {
        m_quit();
    }
}

}
