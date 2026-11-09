// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Drivers of one daemon module. Same interface as the daemon SysFsDriverManager (used by providers),
 * kernel events come from IModuleHost: one udev filter per subsystem (properties of all drivers merged,
 * so every driver gets the same properties) and subsystem "module" (kernel module load/unload: the drivers of the
 * kernel module are initialized again or cleaned).
 */

#include "SysFsDriver.h"

#include <IModuleHost.h>

#include <map>
#include <memory>

namespace LenovoLegionDaemon::Kit {

class SysFsDriverManager
{
public:

    using exception_T = DriverException;

    enum ERROR_CODES : int {
        DRIVER_NOT_FOUND        = -1,
        DRIVER_ALREADY_LOADED   = -2,
    };

    static constexpr const char* MODULE_SUBSYSTEM = "module";

public:

    explicit SysFsDriverManager(Module::IModuleHost& host);
    ~SysFsDriverManager();

    void addDriver(std::unique_ptr<SysFsDriver> driver);

    /*
     * Init and validate drivers, register kernel event filters. Driver which fails is logged and cleaned.
     */
    void initDrivers();
    void cleanDrivers();

    /*
     * Kernel event from the host
     */
    void handleKernelEvent(const Module::KernelEvent& event);

    /*
     * Receiver of driver events
     */
    void setEventSink(SysFsDriver::EventSink sink);

    /*
     * Interface used by providers (same as the daemon SysFsDriverManager)
     */
    void  blockKernelEvent(const QString& driverName,bool block);
    void  refreshDriver(const QString& driverName);

    const SysFsDriver::DescriptorType&          getDriverDesriptor(const QString& driverName) const;
    const SysFsDriver::DescriptorsInVectorType& getDriverDescriptorsInVector(const QString& driverName) const;

    void processAllUdevEvents(int timeoutInMiliseconds);

    bool isLoaded(const QString& driverName) const;

private:

    SysFsDriver& driver(const QString& driverName) const;
    void         initDriver(SysFsDriver& driver);

    /*
     * Drivers of a kernel module: of this manager, all loaded, initialized again (true when one was loaded now)
     */
    bool         hasModule(const QString& module) const;
    bool         moduleLoaded(const QString& module) const;
    bool         retryModule(const QString& module);
    void         startRetry(const QString& module);
    void         stopRetry();

    /*
     * Kernel module added: drivers not loaded yet are initialized again every RETRY_INTERVAL until all are loaded
     * or RETRY_COUNT attempts (a driver of a feature the notebook does not have never loads), the host is told
     * the module waits meanwhile (IModule::onKernelModule is delivered when no daemon module waits)
     */
    void         moduleAdded(const QString& module);
    void         moduleRemoved(const QString& module);

    static constexpr std::chrono::milliseconds RETRY_INTERVAL{250};
    static constexpr int                       RETRY_COUNT = 12;

private:

    Module::IModuleHost&                             m_host;
    std::map<QString,std::unique_ptr<SysFsDriver>>   m_drivers;
    SysFsDriver::EventSink                           m_sink;
    std::map<QString,int>                            m_retries;          // kernel module -> attempts left
    Module::TimerId                                  m_retryTimer = Module::INVALID_TIMER_ID;
};

}
