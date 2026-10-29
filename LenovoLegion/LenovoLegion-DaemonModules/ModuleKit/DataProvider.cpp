// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProvider.h"

namespace LenovoLegionDaemon::Kit {

DataProvider::DataProvider(quint8 dataType) :
    m_dataType(dataType)
{}

DataProvider::~DataProvider()
{
    while(!m_timers.empty())
    {
        killTimer(m_timers.begin()->first);
    }
}

void DataProvider::attach(Module::IModuleHost *host, EventSink sink)
{
    m_host = host;
    m_sink = std::move(sink);
}

void DataProvider::dataProviderEvent(const SubsystemEvent &event)
{
    if(m_sink)
    {
        m_sink(event);
    }
}

int DataProvider::startTimer(std::chrono::milliseconds interval)
{
    if(m_host == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_DATA,"Timers are not available, provider is not attached !");
    }

    const int id = m_nextTimerId++;

    m_timers[id] = m_host->addTimer(interval,[this,id] {
        QTimerEvent event(id);
        timerEvent(&event);
    });

    return id;
}

void DataProvider::killTimer(int timerId)
{
    const auto it = m_timers.find(timerId);

    if(it == m_timers.end())
    {
        return;
    }

    if(m_host != nullptr)
    {
        m_host->removeTimer(it->second);
    }

    m_timers.erase(it);
}

}
