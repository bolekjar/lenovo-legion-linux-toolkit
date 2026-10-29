// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Basic types shared by the framework and the modules.
 */

#include <QByteArray>
#include <QMap>
#include <QSet>
#include <QString>
#include <QVariant>

#include <functional>
#include <stdexcept>

/*
 * Types used across the module boundary (.so built with -fvisibility=hidden), their typeinfo and vtables
 * must be visible so exceptions and virtual calls work between the daemon and modules
 */
#define LEGION_MODULE_VISIBLE __attribute__((visibility("default")))

namespace LenovoLegionDaemon::Module {

/*
 * Error thrown by modules, the framework catches std::exception (modules do not depend on BJLibs)
 */
class LEGION_MODULE_VISIBLE Error : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

/*
 * Kernel (udev) events
 */
struct KernelEvent
{
    /*
     * Module registers filters, the framework delivers only events of the registered subsystems.
     * Loading/unloading of kernel modules is delivered as any other event, register subsystem "module".
     */
    struct Filter
    {
        QString       m_subSystem;      // e.g. "legion-intel-msr", "module"
        QSet<QString> m_properties;     // udev properties copied to Event::m_properties, e.g. "EVENT_TYPE"
    };

    QString                m_action;     // "add", "remove", "change", ...
    QString                m_driver;
    QString                m_sysName;    // for subsystem "module" the kernel module name
    QString                m_subSystem;
    QString                m_devPath;
    QMap<QString,QString>  m_properties;
};


/*
 * Logger of the framework, modules must not use LoggerHolder (each .so would have its own instance)
 */
class LEGION_MODULE_VISIBLE ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void trace(const QString& message)   = 0;
    virtual void debug(const QString& message)   = 0;
    virtual void info(const QString& message)    = 0;
    virtual void warning(const QString& message) = 0;
    virtual void error(const QString& message)   = 0;
};


/*
 * Persistent settings of one module, the framework selects the group (Info::settingsGroup)
 */
class LEGION_MODULE_VISIBLE ISettingsStorage
{
public:
    virtual ~ISettingsStorage() = default;

    virtual bool     contains(const QString& key) const                           = 0;
    virtual QVariant value(const QString& key,const QVariant& def = {}) const     = 0;
    virtual void     setValue(const QString& key,const QVariant& value)           = 0;
    virtual void     remove(const QString& key)                                   = 0;
};


/*
 * Timer of the framework, callback is called from the daemon event loop.
 * The framework catches exceptions from the callback (exception must not go through Qt code).
 */
using TimerId       = int;
using TimerCallback = std::function<void()>;

constexpr TimerId INVALID_TIMER_ID = -1;

}
