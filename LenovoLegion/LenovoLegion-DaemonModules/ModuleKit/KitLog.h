// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Logging in daemon modules, messages go to the logger of the framework (IModuleHost::logger).
 * Same macros as the daemon (LOG_T/D/I/W/E), so the code of drivers and providers stays unchanged.
 * Every module is its own .so, so one logger per library is enough.
 */

#include <ModuleTypes.h>

#include <QString>

namespace LenovoLegionDaemon::Kit {

enum class LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

void setLogger(Module::ILogger* logger);
void log(LogLevel level,const QString& message);

}

#define LOG_T(x) ::LenovoLegionDaemon::Kit::log(::LenovoLegionDaemon::Kit::LogLevel::TRACE,  QString(x));
#define LOG_D(x) ::LenovoLegionDaemon::Kit::log(::LenovoLegionDaemon::Kit::LogLevel::DEBUG,  QString(x));
#define LOG_I(x) ::LenovoLegionDaemon::Kit::log(::LenovoLegionDaemon::Kit::LogLevel::INFO,   QString(x));
#define LOG_W(x) ::LenovoLegionDaemon::Kit::log(::LenovoLegionDaemon::Kit::LogLevel::WARNING,QString(x));
#define LOG_E(x) ::LenovoLegionDaemon::Kit::log(::LenovoLegionDaemon::Kit::LogLevel::ERROR,  QString(x));
