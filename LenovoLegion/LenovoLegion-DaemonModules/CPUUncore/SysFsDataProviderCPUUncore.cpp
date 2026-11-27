// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUUncore.h"

#include <Drivers/SysFsDriverCPUIntelUncore.h>

#include <KitLog.h>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUUncore::SysFsDataProviderCPUUncore(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

legion::messages::CPUUncore SysFsDataProviderCPUUncore::read() const
{
    legion::messages::CPUUncore uncore;

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUIntelUncore::DRIVER_NAME))
    {
        return uncore;
    }

    const SysFsDriverCPUIntelUncore::Uncore driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelUncore::DRIVER_NAME));

    if(driver.m_domains.empty())
    {
        return uncore;
    }

    const auto& domain = driver.m_domains.front();

    uncore.set_min_freq(getData(domain.m_minFreq).trimmed().toUInt());
    uncore.set_max_freq(getData(domain.m_maxFreq).trimmed().toUInt());
    uncore.set_min_limit(getData(domain.m_initialMinFreq).trimmed().toUInt());
    uncore.set_max_limit(getData(domain.m_initialMaxFreq).trimmed().toUInt());

    if(!domain.m_currentFreq.empty())
    {
        uncore.set_current_freq(getData(domain.m_currentFreq).trimmed().toUInt());
    }

    return uncore;
}

QByteArray SysFsDataProviderCPUUncore::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    return serializeMessage(read());
}

QByteArray SysFsDataProviderCPUUncore::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::CPUUncore uncore = parseMessage<legion::messages::CPUUncore>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(uncore,Origin::REQUEST);

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUIntelUncore::DRIVER_NAME) || (!uncore.has_min_freq() && !uncore.has_max_freq()))
    {
        return {};
    }

    const SysFsDriverCPUIntelUncore::Uncore driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelUncore::DRIVER_NAME));

    /*
     * The kernel keeps min <= max: max, min and max again (the order of the new values is not known), all domains
     */
    for(const auto& domain : driver.m_domains)
    {
        if(uncore.has_max_freq()) setData(domain.m_maxFreq,uncore.max_freq());
        if(uncore.has_min_freq()) setData(domain.m_minFreq,uncore.min_freq());
        if(uncore.has_max_freq()) setData(domain.m_maxFreq,uncore.max_freq());
    }

    return {};
}

QByteArray SysFsDataProviderCPUUncore::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUUncore>(data,[this,origin](legion::messages::CPUUncore& uncore) { settable(uncore,origin); });
}

void SysFsDataProviderCPUUncore::settable(legion::messages::CPUUncore &uncore, Origin origin) const
{
    /*
     * Read-only values; min and max in the limits of the hardware and min <= max
     */
    uncore.clear_current_freq();
    uncore.clear_min_limit();
    uncore.clear_max_limit();

    const legion::messages::CPUUncore current = read();

    if(!current.has_max_limit())
    {
        if(uncore.has_min_freq() || uncore.has_max_freq())
        {
            skipField(origin,"intel uncore frequency","driver not available");
        }

        uncore.Clear();
        return;
    }

    const uint32_t min = uncore.has_min_freq() ? uncore.min_freq() : current.min_freq();
    const uint32_t max = uncore.has_max_freq() ? uncore.max_freq() : current.max_freq();
    const QString  range = QString("range <%1,%2> kHz, min <= max").arg(current.min_limit()).arg(current.max_limit());

    if(min < current.min_limit() || min > current.max_limit() || max < current.min_limit() || max > current.max_limit() || min > max)
    {
        skipField(origin,QString("intel uncore frequency min=%1 max=%2").arg(min).arg(max),range);
        uncore.clear_min_freq();
        uncore.clear_max_freq();
    }
}

}
