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

#include <SysFsDriver.h>

#include <libudev.h>

#include <poll.h>

namespace LenovoLegionDaemon {

const  SysFsDriver::KernelEvent::Filter SysFsDriverManager::MODULE_SUBSYSTEM_EVENT_FILTER = { "module" ,{}};

SysFsDriverManager::SysFsDriverManager(QObject *parent)
    : QObject{parent},
    m_udev(udev_new()),
    m_mon(nullptr),
    m_socketNotifier(nullptr)
{
    if(m_udev == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::UDEV_INITIALIZED_ERROR,"Udev new error !");
    }

    m_mon = udev_monitor_new_from_netlink(m_udev, SysFsDriver::KernelEvent::Filter::NAME.data());

    if(m_mon == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::UDEV_INITIALIZED_ERROR,"Udev monitor error !");
    }

    if(udev_monitor_enable_receiving(m_mon) < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::UDEV_INITIALIZED_ERROR,"Udev monitor enable error !");
    }

    m_socketNotifier = new QSocketNotifier(udev_monitor_get_fd(m_mon), QSocketNotifier::Read, this);

    connect(m_socketNotifier,&QSocketNotifier::activated,this,&SysFsDriverManager::onDataReceived);

    /*
     * Add module subsystem filter
     */
    addUdevMonitorFilter(MODULE_SUBSYSTEM_EVENT_FILTER);
}

SysFsDriverManager::~SysFsDriverManager()
{
    /* Disable socket notifier first to prevent any callbacks during destruction */
    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        // Qt will delete it automatically since it has 'this' as parent
    }

    /* free udev monitor */
    if (m_mon) {
        udev_monitor_unref(m_mon);
    }

    /* free udev */
    if (m_udev) {
        udev_unref(m_udev);
    }

    cleanDrivers();
}

void SysFsDriverManager::addDriver(SysFsDriver *driver)
{
    if(driver == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver is nullptr !");
    }

    if(m_drivers.insert({driver->m_name,driver}).second == false)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_ALREADY_LOADED,"Driver already loaded !");
    };
}

void SysFsDriverManager::initDrivers()
{
    for(auto& driver : m_drivers)
    {
        driver.second->init();
        driver.second->validate();

        connect(driver.second,&SysFsDriver::kernelEvent,this,&SysFsDriverManager::onKernelEvent);

        addUdevMonitorFilter(driver.second->m_filter);
    }
}

void SysFsDriverManager::cleanDrivers()
{
    for(auto& driver : m_drivers)
    {
        driver.second->clean();
        delete driver.second;  // Free driver memory
    }

    m_drivers.clear();
}

void SysFsDriverManager::blockKernelEvent(const QString &driverName, bool block)
{
    try {
        m_drivers.at(driverName)->blockKernelEvent(block);
    } catch (const std::out_of_range& ex) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver not found !");
    }
}

void SysFsDriverManager::blockSignals(const QString &driverName,bool block)
{
    try {
        m_drivers.at(driverName)->blockSignals(block);
    } catch (const std::out_of_range& ex) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver not found !");
    }
}

void SysFsDriverManager::refreshDriver(const QString &driverName)
{
    try {
        m_drivers.at(driverName)->init();
        m_drivers.at(driverName)->validate();
    } catch (const std::out_of_range& ex) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver not found !");
    }
}

const SysFsDriver::DescriptorType&  SysFsDriverManager::getDriverDesriptor(const QString &driverName) const
{
    try {
        return m_drivers.at(driverName)->desriptor();
    } catch (const std::out_of_range& ex) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver not found !");
    }
}

const SysFsDriver::DescriptorsInVectorType &SysFsDriverManager::getDriverDescriptorsInVector(const QString &driverName) const
{
    try {
        return m_drivers.at(driverName)->descriptorsInVector();
    } catch (const std::out_of_range& ex) {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver not found !");
    }
}

void SysFsDriverManager::processAllUdevEvents(int timeoutInMiliseconds)
{
    // Process ALL pending SocketNotifier events
    // Use poll to check if FD is ready
    pollfd pfd;
    pfd.fd = m_socketNotifier->socket();
    pfd.events = POLLIN;
    pfd.revents = 0;


    while (true) {
        pfd.revents = 0;
        int ret = poll(&pfd, 1, timeoutInMiliseconds);

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
    struct udev_device *dev = udev_monitor_receive_device(m_mon);

    if(dev != nullptr)
    {
        SysFsDriver::KernelEvent::Event event = {
            .m_action       = udev_device_get_action(dev)   ,
            .m_driver       = udev_device_get_driver(dev)   ,
            .m_sysName      = udev_device_get_sysname(dev)  ,
            .m_subSystem    = udev_device_get_subsystem(dev),
            .m_devPath      = udev_device_get_devpath(dev)  ,
            .m_properties   = {}
        };

         LOG_D(QString(__PRETTY_FUNCTION__) + ": Kernel event received ACTION=" + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

        if(event.m_subSystem == MODULE_SUBSYSTEM_EVENT_FILTER.m_subSystem)
        {
            for(auto& driver : m_drivers)
            {
                if(driver.second->m_module == event.m_sysName)
                {
                    if(event.m_action == "add")
                    {
                        driver.second->init();
                        driver.second->validate();

                        emit moduleSubsystem({
                            .m_moduleName = driver.second->m_module,
                            .m_action     = ModuleSubsystemEvent::Action::ADD
                        });
                    }

                    if(event.m_action == "remove")
                    {
                        driver.second->clean();

                        emit moduleSubsystem({
                            .m_moduleName = driver.second->m_module,
                            .m_action     = ModuleSubsystemEvent::Action::REMOVE
                        });
                    }
                }
            }
        }
        else
        {
            for (auto& driver : m_drivers)
            {
                if(driver.second->m_filter.m_subSystem == event.m_subSystem)
                {
                    for (const auto & property :driver.second->m_filter.m_properties)
                    {
                        event.m_properties[property] = udev_device_get_property_value(dev, property.toStdString().c_str());
                    }

                    driver.second->handleKernelEvent(event);
                }
            }
        }

        udev_device_unref(dev);
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

void SysFsDriverManager::onKernelEvent(const SysFsDriver::SubsystemEvent &event)
{
    emit kernelEvent(event);
}

void SysFsDriverManager::addUdevMonitorFilter(const SysFsDriver::KernelEvent::Filter &filter)
{
    if(filter.m_subSystem.size() > 0)
    {
        if(udev_monitor_filter_add_match_subsystem_devtype(m_mon, filter.m_subSystem.toStdString().c_str(), nullptr) < 0)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::UDEV_INITIALIZED_ERROR,"Udev monitor filter add error !");
        }
    }
}

void SysFsDriverManager::reconnectUdevMonitor()
{
    LOG_W(QString(__PRETTY_FUNCTION__) + ": Reconnecting udev monitor");

    // Disconnect and delete old socket notifier
    if (m_socketNotifier) {
        m_socketNotifier->setEnabled(false);
        disconnect(m_socketNotifier, &QSocketNotifier::activated, this, &SysFsDriverManager::onDataReceived);
        delete m_socketNotifier;
        m_socketNotifier = nullptr;
    }

    // Clean up old monitor and udev
    if (m_mon) {
        udev_monitor_unref(m_mon);
        m_mon = nullptr;
    }

    if (m_udev) {
        udev_unref(m_udev);
        m_udev = nullptr;
    }

    // Recreate udev
    m_udev = udev_new();
    if (m_udev == nullptr) {
        LOG_E(QString(__PRETTY_FUNCTION__) + ": Failed to recreate udev");

        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev new error !");
    }

    // Recreate monitor
    m_mon = udev_monitor_new_from_netlink(m_udev, SysFsDriver::KernelEvent::Filter::NAME.data());
    if (m_mon == nullptr) {
        LOG_E(QString(__PRETTY_FUNCTION__) + ": Failed to recreate udev monitor");
        udev_unref(m_udev);
        m_udev = nullptr;

        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev monitor error !");
    }

    // Re-add all filters
    // Add module subsystem filter first
    addUdevMonitorFilter(MODULE_SUBSYSTEM_EVENT_FILTER);

    // Add driver-specific filters
    for (const auto& driver : m_drivers) {
        addUdevMonitorFilter(driver.second->m_filter);
    }

    // Enable receiving
    if (udev_monitor_enable_receiving(m_mon) < 0) {
        LOG_E(QString(__PRETTY_FUNCTION__) + ": Failed to enable receiving on new monitor");
        udev_monitor_unref(m_mon);
        udev_unref(m_udev);
        m_mon = nullptr;
        m_udev = nullptr;

        THROW_EXCEPTION(exception_T, ERROR_CODES::UDEV_INITIALIZED_ERROR, "Udev monitor enable error !");
    }

    // Create new socket notifier
    m_socketNotifier = new QSocketNotifier(udev_monitor_get_fd(m_mon), QSocketNotifier::Read, this);
    connect(m_socketNotifier, &QSocketNotifier::activated, this, &SysFsDriverManager::onDataReceived);

    LOG_W(QString(__PRETTY_FUNCTION__) + ": Udev monitor reconnected successfully");
}

}
