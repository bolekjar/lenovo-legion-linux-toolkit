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
#include <map>
#include <set>
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

    /*
     * Removes kernel event and kernel module listeners of the owner, the owner does not wait for kernel modules
     */
    void removeKernelEventListeners(const QObject* owner);

    /*
     * Kernel modules (subsystem "module"): owners initializing the drivers of a kernel module report it as waiting,
     * kernel module listeners get loaded=true once no owner waits (after the "add" event was delivered to all
     * listeners), loaded=false at once after the "remove" event. Listener must not throw.
     */
    using KernelModuleListener = std::function<void(const QString& kernelModule,bool loaded)>;

    static constexpr const char* MODULE_SUBSYSTEM = "module";

    void addKernelModuleListener(const QObject* owner,KernelModuleListener listener);
    void setKernelModuleWaiting(const QObject* owner,const QString& kernelModule,bool waiting);

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

    /*
     * Kernel modules: loaded (announced to the listeners), pending (added, owners still waiting), removed
     */
    enum class KernelModuleState { REMOVED, PENDING, LOADED };

    struct KernelModule
    {
        KernelModuleState          m_state = KernelModuleState::REMOVED;
        std::set<const QObject*>   m_waiting;
    };

    /*
     * Announce loaded when the module is pending and no owner waits (not while the "add" event is dispatched)
     */
    void checkKernelModuleLoaded(const QString& kernelModule);
    void announceKernelModule(const QString& kernelModule,bool loaded);

    std::vector<std::pair<const QObject*,KernelModuleListener>> m_kernelModuleListeners;
    std::map<QString,KernelModule>                              m_kernelModules;
    QString                                                     m_dispatchedKernelModule;
};

}
