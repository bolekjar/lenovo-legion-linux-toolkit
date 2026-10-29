// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverManager.h"

#include <Core/LoggerHolder.h>

#include <QSocketNotifier>
#include <QDeadlineTimer>
#include <QTimer>
#include <QScopeGuard>

#include <libudev.h>

#include <poll.h>

namespace LenovoLegionDaemon {

SysFsDriverManager::SysFsDriverManager(QObject *parent)
    : QObject{parent},
    m_udev(nullptr),
    m_mon(nullptr),
    m_socketNotifier(nullptr)
{
    try {
        createUdevMonitor();
    } catch (...) {
        destroyUdevMonitor();
        throw;
    }
}

SysFsDriverManager::~SysFsDriverManager()
{
    destroyUdevMonitor();
}

void SysFsDriverManager::addKernelEventListener(const QObject *owner, const Module::KernelEvent::Filter &filter, KernelEventListener listener)
{
    addUdevMonitorFilter(filter.m_subSystem);

    m_kernelEventListeners.push_back({ .m_owner = owner, .m_filter = filter, .m_listener = std::move(listener) });
}

void SysFsDriverManager::removeKernelEventListeners(const QObject *owner)
{
    std::erase_if(m_kernelEventListeners,[owner](const KernelEventListenerEntry& entry) { return entry.m_owner == owner; });
}

void SysFsDriverManager::processAllUdevEvents(int timeoutInMiliseconds)
{
    if (m_socketNotifier == nullptr) {
        LOG_W(QString(__PRETTY_FUNCTION__) + ": Udev monitor is not available, events are not processed");
        return;
    }

    // Process ALL pending SocketNotifier events
    // Use poll to check if FD is ready
    pollfd pfd;
    pfd.fd = m_socketNotifier->socket();
    pfd.events = POLLIN;
    pfd.revents = 0;

    // Timeout is for the whole processing, unrelated kernel events must not extend
    // the wait, the event loop is blocked meanwhile. After the deadline only already
    // pending events are processed.
    const QDeadlineTimer deadline(timeoutInMiliseconds);

    while (true) {
        pfd.revents = 0;
        int ret = poll(&pfd, 1, static_cast<int>(deadline.remainingTime()));

        // Break conditions:
        if (ret == 0) {
            // Timeout - no more data ready
            break;
        }

        if (ret < 0) {
            LOG_E(QString(__PRETTY_FUNCTION__) + ": Poll error");
            break;
        }

        // Check for unrecoverable error events first
        if (pfd.revents & POLLERR) {
            LOG_E(QString(__PRETTY_FUNCTION__) + ": Socket error - attempting reconnection");
            reconnectUdevMonitor();
            break;
        }

        if (pfd.revents & POLLNVAL) {
            LOG_E(QString(__PRETTY_FUNCTION__) + ": Invalid file descriptor - attempting reconnection");
            reconnectUdevMonitor();
            break;
        }

        // Read any available data first (even if POLLHUP is also set)
        if (pfd.revents & POLLIN) {
            onDataReceived(pfd.fd);

            // Monitor was recreated (or is not available) while processing, stop using the old fd
            if (m_socketNotifier == nullptr || m_socketNotifier->socket() != pfd.fd) {
                break;
            }
        }

        // Check for hangup AFTER reading data
        if (pfd.revents & POLLHUP) {
            LOG_E(QString(__PRETTY_FUNCTION__) + ": Udev monitor disconnected - attempting reconnection");
            reconnectUdevMonitor();
            break;
        }
    }
}

void SysFsDriverManager::onDataReceived(int)
{
    if (m_mon == nullptr) {
        return;
    }

    struct udev_device *dev = udev_monitor_receive_device(m_mon);

    if(dev != nullptr)
    {
        // Release device also when a listener throws
        const auto unrefDevice = qScopeGuard([dev] { udev_device_unref(dev); });

        const QString action    = udev_device_get_action(dev);
        const QString driver    = udev_device_get_driver(dev);
        const QString sysName   = udev_device_get_sysname(dev);
        const QString subSystem = udev_device_get_subsystem(dev);
        const QString devPath   = udev_device_get_devpath(dev);

        LOG_D(QString(__PRETTY_FUNCTION__) + ": Kernel event received ACTION=" + action + ", DRIVER=" + driver + ", SYSNAME=" + sysName + ", SUBSYSTEM=" + subSystem + ", DEVPATH=" + devPath);

        /*
         * Daemon modules, copy of the list, a listener may add or remove listeners
         */
        const auto listeners = m_kernelEventListeners;

        for (const auto& entry : listeners)
        {
            if(entry.m_filter.m_subSystem != subSystem)
            {
                continue;
            }

            Module::KernelEvent moduleEvent {
                .m_action     = action,
                .m_driver     = driver,
                .m_sysName    = sysName,
                .m_subSystem  = subSystem,
                .m_devPath    = devPath,
                .m_properties = {}
            };

            for (const auto& property : entry.m_filter.m_properties)
            {
                moduleEvent.m_properties[property] = udev_device_get_property_value(dev, property.toStdString().c_str());
            }

            entry.m_listener(moduleEvent);
        }
    }
    else
    {
        // NULL device can mean:
        // 1. No data available (EAGAIN) - normal, just return
        // 2. Socket error or disconnect - need to check
        
        // Use poll to check the actual socket state
        pollfd pfd;
        pfd.fd = udev_monitor_get_fd(m_mon);
        pfd.events = POLLIN;
        pfd.revents = 0;
        
        int ret = poll(&pfd, 1, 0);
        
        // Check for error conditions
        if (ret > 0 && (pfd.revents & (POLLERR | POLLHUP | POLLNVAL))) {
            if (pfd.revents & POLLERR) {
                LOG_E(QString(__PRETTY_FUNCTION__) + ": Socket error detected");
            }
            if (pfd.revents & POLLHUP) {
                LOG_E(QString(__PRETTY_FUNCTION__) + ": Socket hangup detected");
            }
            if (pfd.revents & POLLNVAL) {
                LOG_E(QString(__PRETTY_FUNCTION__) + ": Invalid socket detected");
            }
            
            // Disconnect is detected - attempt reconnection
            reconnectUdevMonitor();
        }
    }
}

void SysFsDriverManager::addUdevMonitorFilter(const QString &subSystem)
{
    // Monitor is not available (failed reconnect), filters of all listeners are added when it is recreated
    if(m_mon == nullptr)
    {
        return;
    }

    if(subSystem.size() > 0)
    {
        if(udev_monitor_filter_add_match_subsystem_devtype(m_mon, subSystem.toStdString().c_str(), nullptr) < 0)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::UDEV_INITIALIZED_ERROR,"Udev monitor filter add error !");
        }
    }
}

void SysFsDriverManager::createUdevMonitor()
{
    m_udev = udev_new();
    if (m_udev == nullptr) {
        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev new error !");
    }

    m_mon = udev_monitor_new_from_netlink(m_udev, UDEV_MONITOR_SOURCE);
    if (m_mon == nullptr) {
        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev monitor error !");
    }

    // Add filters of daemon modules (empty during construction)
    for (const auto& entry : m_kernelEventListeners) {
        addUdevMonitorFilter(entry.m_filter.m_subSystem);
    }

    if (udev_monitor_enable_receiving(m_mon) < 0) {
        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev monitor enable error !");
    }

    m_socketNotifier = new QSocketNotifier(udev_monitor_get_fd(m_mon), QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &SysFsDriverManager::onDataReceived);
}

void SysFsDriverManager::destroyUdevMonitor() noexcept
{
    // Notifier may be the sender of the currently running onDataReceived, delete it later
    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &SysFsDriverManager::onDataReceived);
        m_socketNotifier->deleteLater();
        m_socketNotifier = nullptr;
    }

    if (m_mon) {
        udev_monitor_unref(m_mon);
        m_mon = nullptr;
    }

    if (m_udev) {
        udev_unref(m_udev);
        m_udev = nullptr;
    }
}

void SysFsDriverManager::reconnectUdevMonitor() noexcept
{
    LOG_W(QString(__PRETTY_FUNCTION__) + ": Reconnecting udev monitor");

    destroyUdevMonitor();

    try {
        createUdevMonitor();

        LOG_W(QString(__PRETTY_FUNCTION__) + ": Udev monitor reconnected successfully");
    } catch (const exception_T& ex) {
        // Keep consistent state (no monitor), kernel events are not received until the next attempt
        destroyUdevMonitor();

        LOG_E(QString(__PRETTY_FUNCTION__) + ": Failed to reconnect udev monitor (" + ex.descriptionInfo().value().c_str() + "), retry in " + QString::number(UDEV_RECONNECT_RETRY_MS) + " ms");

        QTimer::singleShot(UDEV_RECONNECT_RETRY_MS, this, [this] {
            if (m_mon == nullptr) {
                reconnectUdevMonitor();
            }
        });
    }
}

}
