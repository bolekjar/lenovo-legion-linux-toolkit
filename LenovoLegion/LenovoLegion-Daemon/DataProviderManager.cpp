// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderManager.h"
#include "SysFsDriverManager.h"


#include <Core/LoggerHolder.h>



namespace  LenovoLegionDaemon {


DataProviderManager::DataProviderManager(SysFsDriverManager* sysFsDriverManager,QObject* parent)  :
    QObject(parent),
    m_sysFsDriverManager(sysFsDriverManager)
{}

void DataProviderManager::addDataProvider(DataProvider *driver)
{
    if(driver == nullptr)
    {
        THROW_EXCEPTION(exception_T,DATA_PROVIDER_NOT_FOUND,"Driver is nullptr !");
    }

    if(m_dataProviders.insert({driver->m_dataType,driver}).second == false)
    {
        THROW_EXCEPTION(exception_T,DATA_PROVIDER_ALREADY_LOADED,"Driver already loaded !");
    };
}

void DataProviderManager::initDataProviders()
{
    for(auto& driver : m_dataProviders)
    {
        driver.second->init();
    }
}

void DataProviderManager::cleanDataProviders()
{
    for(auto& driver : m_dataProviders)
    {
        driver.second->clean();
    }

    m_dataProviders.clear();
}

DataProvider& DataProviderManager::getDataProvider(const quint8 dataType){

    if(m_dataProviders.count(dataType) == 0)
    {
        THROW_EXCEPTION(exception_T,DATA_PROVIDER_NOT_FOUND,"Data provider not found !");
    }

    return *m_dataProviders.at(dataType);
}

void DataProviderManager::forEachDataProviderDo(const std::function<void (DataProvider &)> &func) const
{
    for(const auto& driver : m_dataProviders)
    {
        func(*driver.second);
    }
}

void DataProviderManager::kernelEventHandler(const SysFsDriver::SubsystemEvent &event)
{
    for(auto& driver : m_dataProviders)
    {
        driver.second->kernelEventHandler(event);
    }
}

}

