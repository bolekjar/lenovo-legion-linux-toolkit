// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../LenovoLegion-DaemonSDK/ModuleTypes.h"

#include <Core/ExceptionBuilder.h>

#include <QObject>
#include <QString>

#include <functional>
#include <vector>

#include <libudev.h>

class QSocketNotifier;

namespace LenovoLegionDaemon {

/*
 * Kernel (udev) event monitor of the framework. Daemon modules register listeners with filters
 * (IModuleHost::addKernelEventFilter), every event of the filter subsystem is delivered to them.
 */
class SysFsDriverManager : public QObject
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(SysFsDriverManager);

    enum ERROR_CODES : int {
        UDEV_INITIALIZED_ERROR  = -3,
    };

public:

    explicit SysFsDriverManager(QObject *parent = nullptr);

    ~SysFsDriverManager();

    /*
     * Kernel event listeners of daemon modules, called for every event of the filter subsystem
     * (also for subsystem "module"). Listener must not throw.
     */
    using KernelEventListener = std::function<void(const Module::KernelEvent& event)>;

    void addKernelEventListener(const QObject* owner,const Module::KernelEvent::Filter& filter,KernelEventListener listener);
    void removeKernelEventListeners(const QObject* owner);

    /*
     * Process already pending kernel events, wait at most timeout in total (blocks the event loop)
     */
    void processAllUdevEvents(int timeoutInMiliseconds);

private slots:

    void onDataReceived(int socket);

private:

    void addUdevMonitorFilter(const QString& subSystem);

    /*
     * Create udev, monitor (with all filters) and socket notifier, throws on failure
     */
    void createUdevMonitor();

    /*
     * Release udev, monitor and socket notifier, safe on partially created monitor
     */
    void destroyUdevMonitor() noexcept;

    /*
     * Recreate broken udev monitor, does not throw, on failure retries after UDEV_RECONNECT_RETRY_MS
     */
    void reconnectUdevMonitor() noexcept;

private:

    static constexpr int         UDEV_RECONNECT_RETRY_MS = 5000;
    static constexpr const char* UDEV_MONITOR_SOURCE     = "kernel";

    struct udev         *m_udev;
    struct udev_monitor *m_mon;

    QSocketNotifier     *m_socketNotifier;

    struct KernelEventListenerEntry
    {
        const QObject*              m_owner;
        Module::KernelEvent::Filter m_filter;
        KernelEventListener         m_listener;
    };

    std::vector<KernelEventListenerEntry> m_kernelEventListeners;
};

}
