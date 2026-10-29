// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <IModule.h>
#include <IModuleHost.h>

#include <QObject>
#include <QPointer>
#include <QString>

#include <functional>
#include <map>

class QTimer;

namespace LenovoLegionDaemon {

class SysFsDriverManager;

/*
 * Implementation of IModuleHost for one daemon module.
 * Every call into the module (kernel events, timers) is wrapped by try/catch, exception is only logged.
 */
class ModuleHost : public QObject,
                   public Module::IModuleHost
{
    Q_OBJECT

public:

    ModuleHost(const QString& moduleName,SysFsDriverManager* sysFsDriverManager,QObject* parent);
    ~ModuleHost() override;

    /*
     * Receiver of kernel events and timers, nullptr stops the delivery
     */
    void setModule(Module::IModule* module);

    /*
     * Remove all timers and kernel event listeners of the module
     */
    void release();

    /*
     * Call into the module, exceptions are logged. Returns false if the call threw.
     */
    bool callModule(const QString& what,const std::function<void()>& call) noexcept;

    const QString& moduleName() const;

    /*
     * IModuleHost
     */
    Module::ILogger& logger() override;

    void addKernelEventFilter(const Module::KernelEvent::Filter& filter) override;
    void blockKernelEvents(bool block) override;
    void processPendingKernelEvents(std::chrono::milliseconds timeout) override;

    void notify(const QByteArray& payload) override;

    Module::TimerId addTimer(std::chrono::milliseconds interval,Module::TimerCallback callback) override;
    void            removeTimer(Module::TimerId id) override;

signals:

    /*
     * Notification of the module for the connected client
     */
    void notification(const QByteArray& payload);

private:

    class Logger : public Module::ILogger
    {
    public:
        explicit Logger(const QString& moduleName);

        void trace(const QString& message)   override;
        void debug(const QString& message)   override;
        void info(const QString& message)    override;
        void warning(const QString& message) override;
        void error(const QString& message)   override;

    private:
        const QString m_prefix;
    };

private:

    const QString                    m_moduleName;
    /*
     * Driver manager may be destroyed before the host (children of Application are deleted in creation order)
     */
    QPointer<SysFsDriverManager>     m_sysFsDriverManager;
    Module::IModule*                 m_module;
    Logger                           m_logger;

    bool                             m_kernelEventsBlocked;

    Module::TimerId                  m_nextTimerId;
    std::map<Module::TimerId,QTimer*> m_timers;
};

}
