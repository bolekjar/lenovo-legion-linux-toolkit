// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "AppSettings.h"

#include "../Settings.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionGui {

std::function<void ()> AppSettings::s_reconnect;

AppSettings::AppSettings(QObject *parent) :
    QObject(parent)
{
    /*
     * Changes from other places (reset, another page) are shown too
     */
    connect(ApplicationSettings::instance(),&ApplicationSettings::settingChanged,this,&AppSettings::changed);
    connect(ApplicationSettings::instance(),&ApplicationSettings::themeChanged,this,&AppSettings::changed);
}

bool AppSettings::startMinimized() const
{
    bool value = false;
    ApplicationSettings::instance()->loadStartMinimized(value);
    return value;
}

bool AppSettings::minimizeToTray() const
{
    bool value = false;
    ApplicationSettings::instance()->loadMinimizeToTray(value);
    return value;
}

bool AppSettings::debugLogging() const
{
    bool value = false;
    ApplicationSettings::instance()->loadAppDebugLogging(value);
    return value;
}

bool AppSettings::traceLogging() const
{
    bool value = false;
    ApplicationSettings::instance()->loadAppTraceLogging(value);
    return value;
}

int AppSettings::theme() const
{
    ApplicationSettings::ThemeType value = ApplicationSettings::ThemeType::NoTheme;
    ApplicationSettings::instance()->loadStylesheetTheme(value);
    return static_cast<int>(value);
}

void AppSettings::setStartMinimized(bool value)
{
    if(value != startMinimized())
    {
        ApplicationSettings::instance()->saveStartMinimized(value);
    }
}

void AppSettings::setMinimizeToTray(bool value)
{
    if(value != minimizeToTray())
    {
        ApplicationSettings::instance()->saveMinimizeToTray(value);
    }
}

void AppSettings::setDebugLogging(bool value)
{
    if(value != debugLogging())
    {
        ApplicationSettings::instance()->saveAppDebugLogging(value);
    }
}

void AppSettings::setTraceLogging(bool value)
{
    if(value != traceLogging())
    {
        ApplicationSettings::instance()->saveAppTraceLogging(value);
    }
}

void AppSettings::setTheme(int value)
{
    if(value < static_cast<int>(ApplicationSettings::ThemeType::NoTheme) || value > static_cast<int>(ApplicationSettings::ThemeType::DarkTheme))
    {
        LOG_W(QString("Invalid theme ").append(QString::number(value)).append(", ignored"));
        return;
    }

    if(value != theme())
    {
        ApplicationSettings::instance()->saveStylesheetTheme(static_cast<ApplicationSettings::ThemeType>(value));
    }
}

void AppSettings::resetToDefaults()
{
    ApplicationSettings::instance()->resetToDefaults();
}

void AppSettings::reconnectDaemon()
{
    if(!s_reconnect)
    {
        LOG_W("Reconnect to the daemon is not available");
        return;
    }

    s_reconnect();
}

void AppSettings::setReconnect(const std::function<void ()> &reconnect)
{
    s_reconnect = reconnect;
}

}
