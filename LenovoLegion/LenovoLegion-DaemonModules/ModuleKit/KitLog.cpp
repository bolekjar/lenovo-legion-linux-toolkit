// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "KitLog.h"

namespace LenovoLegionDaemon::Kit {

namespace {

Module::ILogger* g_logger = nullptr;

}

void setLogger(Module::ILogger *logger)
{
    g_logger = logger;
}

void log(LogLevel level, const QString &message)
{
    if(g_logger == nullptr)
    {
        return;
    }

    switch (level) {
    case LogLevel::TRACE:   g_logger->trace(message);   break;
    case LogLevel::DEBUG:   g_logger->debug(message);   break;
    case LogLevel::INFO:    g_logger->info(message);    break;
    case LogLevel::WARNING: g_logger->warning(message); break;
    case LogLevel::ERROR:   g_logger->error(message);   break;
    }
}

}
