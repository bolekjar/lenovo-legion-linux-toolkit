// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Interface of the framework for one module. Every module gets its own host instance,
 * so all calls are related to the calling module (kernel events, notifications, timers).
 * All methods must be called from the daemon event loop thread.
 */

#include "ModuleTypes.h"

#include <chrono>

namespace LenovoLegionDaemon::Module {

class LEGION_MODULE_VISIBLE IModuleHost
{
public:
    virtual ~IModuleHost() = default;

    /*
     * Logger of the framework
     */
    virtual ILogger& logger() = 0;


    /*
     * Kernel events
     */

    /* Register udev filter, events are delivered by IModule::onKernelEvent */
    virtual void addKernelEventFilter(const KernelEvent::Filter& filter) = 0;

    /* Block delivery of kernel events to this module (e.g. events caused by own write) */
    virtual void blockKernelEvents(bool block) = 0;

    /* Process already pending kernel events, wait at most timeout in total (blocks the event loop) */
    virtual void processPendingKernelEvents(std::chrono::milliseconds timeout) = 0;


    /*
     * Notification to the connected client, payload is defined by the module.
     * Transitional: serialized legion::messages::Notification, sent as today by ProtocolProcessorNotifier.
     */
    virtual void notify(const QByteArray& payload) = 0;


    /*
     * Timers (replaces QObject::startTimer in modules, exceptions are caught by the framework)
     */
    virtual TimerId addTimer(std::chrono::milliseconds interval,TimerCallback callback) = 0;
    virtual void    removeTimer(TimerId id) = 0;


    /*
     * Shared system services (ICpuList, ILegionAttributes, ILegionEvents) will be added
     * in the "system services" step, the pilot module IntelMSR does not need them.
     */
};


/*
 * Blocks kernel events of the module for the lifetime of the object and processes events
 * caused by the own write before unblocking (replaces blockKernelEvent + qScopeGuard + processAllUdevEvents).
 */
class KernelEventBlocker
{
public:
    explicit KernelEventBlocker(IModuleHost& host,std::chrono::milliseconds processTimeout = std::chrono::milliseconds(100)) :
        m_host(host),
        m_processTimeout(processTimeout)
    {
        m_host.blockKernelEvents(true);
    }

    ~KernelEventBlocker()
    {
        m_host.processPendingKernelEvents(m_processTimeout);
        m_host.blockKernelEvents(false);
    }

    KernelEventBlocker(const KernelEventBlocker&)            = delete;
    KernelEventBlocker& operator=(const KernelEventBlocker&) = delete;

private:

    IModuleHost&                    m_host;
    const std::chrono::milliseconds m_processTimeout;
};

}
