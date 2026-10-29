// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverManager.h"

namespace LenovoLegionDaemon::Kit {

SysFsDriverManager::SysFsDriverManager(Module::IModuleHost &host) :
    m_host(host)
{}

SysFsDriverManager::~SysFsDriverManager()
{
    cleanDrivers();
}

void SysFsDriverManager::addDriver(std::unique_ptr<SysFsDriver> driver)
{
    if(driver == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver is nullptr !");
    }

    const QString name = driver->m_name;

    driver->setEventSink([this](const SysFsDriver::SubsystemEvent& event) {
        if(m_sink)
        {
            m_sink(event);
        }
    });

    if(!m_drivers.emplace(name,std::move(driver)).second)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_ALREADY_LOADED,"Driver " + name.toStdString() + " already loaded !");
    }
}

void SysFsDriverManager::initDriver(SysFsDriver &driver)
{
    try {
        driver.init();
        driver.validate();
    } catch (const Exception& ex) {
        LOG_E(QString("Driver ").append(driver.m_name).append(" initialization failed (").append(ex.what()).append("), driver is not available"));
        driver.clean();
    }
}

void SysFsDriverManager::initDrivers()
{
    /*
     * One filter per subsystem, properties of all drivers merged
     */
    std::map<QString,QSet<QString>> filters;

    for(auto& driver : m_drivers)
    {
        initDriver(*driver.second);

        if(!driver.second->m_filter.m_subSystem.isEmpty())
        {
            filters[driver.second->m_filter.m_subSystem].unite(driver.second->m_filter.m_properties);
        }
    }

    filters[MODULE_SUBSYSTEM];

    for(const auto& filter : filters)
    {
        m_host.addKernelEventFilter({ .m_subSystem = filter.first, .m_properties = filter.second });
    }
}

void SysFsDriverManager::cleanDrivers()
{
    for(auto& driver : m_drivers)
    {
        driver.second->clean();
    }
}

void SysFsDriverManager::handleKernelEvent(const Module::KernelEvent &event)
{
    if(event.m_subSystem == MODULE_SUBSYSTEM)
    {
        for(auto& driver : m_drivers)
        {
            if(driver.second->m_module != event.m_sysName)
            {
                continue;
            }

            if(event.m_action == "add")
            {
                initDriver(*driver.second);
            }

            if(event.m_action == "remove")
            {
                driver.second->clean();
            }
        }

        return;
    }

    for(auto& driver : m_drivers)
    {
        if(driver.second->m_filter.m_subSystem == event.m_subSystem)
        {
            driver.second->handleKernelEvent(event);
        }
    }
}

void SysFsDriverManager::setEventSink(SysFsDriver::EventSink sink)
{
    m_sink = std::move(sink);
}

void SysFsDriverManager::blockKernelEvent(const QString &driverName, bool block)
{
    driver(driverName).blockKernelEvent(block);
}

void SysFsDriverManager::refreshDriver(const QString &driverName)
{
    initDriver(driver(driverName));
}

const SysFsDriver::DescriptorType &SysFsDriverManager::getDriverDesriptor(const QString &driverName) const
{
    return driver(driverName).desriptor();
}

const SysFsDriver::DescriptorsInVectorType &SysFsDriverManager::getDriverDescriptorsInVector(const QString &driverName) const
{
    return driver(driverName).descriptorsInVector();
}

void SysFsDriverManager::processAllUdevEvents(int timeoutInMiliseconds)
{
    m_host.processPendingKernelEvents(std::chrono::milliseconds(timeoutInMiliseconds));
}

bool SysFsDriverManager::isLoaded(const QString &driverName) const
{
    const auto it = m_drivers.find(driverName);

    return it != m_drivers.end() && it->second->isLoaded();
}

SysFsDriver &SysFsDriverManager::driver(const QString &driverName) const
{
    const auto it = m_drivers.find(driverName);

    if(it == m_drivers.end())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_FOUND,"Driver " + driverName.toStdString() + " not found !");
    }

    return *it->second;
}

}
