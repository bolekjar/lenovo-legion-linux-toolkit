// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderIGPUFrequency.h"

#include <Drivers/SysFsDriverIGpuFrequency.h>

#include <KitLog.h>

#include <algorithm>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderIGPUFrequency::SysFsDataProviderIGPUFrequency(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

legion::messages::IGpuFrequency SysFsDataProviderIGPUFrequency::read() const
{
    legion::messages::IGpuFrequency frequency;

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverIGpuFrequency::DRIVER_NAME))
    {
        return frequency;
    }

    const SysFsDriverIGpuFrequency::Frequency driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverIGpuFrequency::DRIVER_NAME));

    const uint32_t min = getData(driver.m_min).trimmed().toUInt();

    frequency.set_driver(driver.m_driver.toStdString());
    frequency.set_min_freq(min);
    frequency.set_max_freq(getData(driver.m_max).trimmed().toUInt());
    frequency.set_max_limit(getData(driver.m_rp0).trimmed().toUInt());

    /*
     * The lower limit is RPn, the driver may allow less (i915 with SLPC: the minimum below RPn)
     */
    frequency.set_min_limit(std::min(min,getData(driver.m_rpn).trimmed().toUInt()));

    frequency.set_boost_supported(!driver.m_boost.empty());

    if(!driver.m_boost.empty())     frequency.set_boost_freq(getData(driver.m_boost).trimmed().toUInt());
    if(!driver.m_efficient.empty()) frequency.set_efficient_freq(getData(driver.m_efficient).trimmed().toUInt());
    if(!driver.m_current.empty())   frequency.set_current_freq(getData(driver.m_current).trimmed().toUInt());

    return frequency;
}

QByteArray SysFsDataProviderIGPUFrequency::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    return serializeMessage(read());
}

QByteArray SysFsDataProviderIGPUFrequency::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::IGpuFrequency frequency = parseMessage<legion::messages::IGpuFrequency>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(frequency,Origin::REQUEST);

    if(!frequency.has_min_freq() && !frequency.has_max_freq() && !frequency.has_boost_freq())
    {
        return {};
    }

    const SysFsDriverIGpuFrequency::Frequency driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverIGpuFrequency::DRIVER_NAME));

    /*
     * The driver keeps min <= boost <= max: max, min, boost and max again (the order of the new values is not known)
     */
    if(frequency.has_max_freq())                           setData(driver.m_max,frequency.max_freq());
    if(frequency.has_min_freq())                           setData(driver.m_min,frequency.min_freq());
    if(frequency.has_boost_freq() && !driver.m_boost.empty()) setData(driver.m_boost,frequency.boost_freq());
    if(frequency.has_max_freq())                           setData(driver.m_max,frequency.max_freq());

    return {};
}

QByteArray SysFsDataProviderIGPUFrequency::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::IGpuFrequency>(data,[this,origin](legion::messages::IGpuFrequency& frequency) { settable(frequency,origin); });
}

void SysFsDataProviderIGPUFrequency::settable(legion::messages::IGpuFrequency &frequency, Origin origin) const
{
    frequency.clear_efficient_freq();
    frequency.clear_current_freq();
    frequency.clear_driver();
    frequency.clear_min_limit();
    frequency.clear_max_limit();
    frequency.clear_boost_supported();

    const legion::messages::IGpuFrequency current = read();

    if(!current.has_max_limit())
    {
        if(frequency.has_min_freq() || frequency.has_max_freq() || frequency.has_boost_freq())
        {
            skipField(origin,"iGPU frequency","no Intel integrated GPU");
        }

        frequency.Clear();
        return;
    }

    if(!current.boost_supported())
    {
        frequency.clear_boost_freq();
    }

    const uint32_t min   = frequency.has_min_freq() ? frequency.min_freq() : current.min_freq();
    const uint32_t max   = frequency.has_max_freq() ? frequency.max_freq() : current.max_freq();
    const QString  range = QString("range <%1,%2> MHz, min <= max").arg(current.min_limit()).arg(current.max_limit());

    if(min < current.min_limit() || max > current.max_limit() || min > max)
    {
        skipField(origin,QString("iGPU frequency min=%1 max=%2").arg(min).arg(max),range);
        frequency.clear_min_freq();
        frequency.clear_max_freq();
        frequency.clear_boost_freq();
        return;
    }

    if(frequency.has_boost_freq() && (frequency.boost_freq() < min || frequency.boost_freq() > max))
    {
        skipField(origin,QString("iGPU boost frequency=%1").arg(frequency.boost_freq()),QString("range <%1,%2> MHz").arg(min).arg(max));
        frequency.clear_boost_freq();
    }
}

}
