// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUTcc.h"

#include <Drivers/SysFsDriverCPUIntelTcc.h>

#include <KitLog.h>

#include <algorithm>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUTcc::SysFsDataProviderCPUTcc(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

legion::messages::CPUTcc SysFsDataProviderCPUTcc::read() const
{
    legion::messages::CPUTcc tcc;

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUIntelTcc::DRIVER_NAME))
    {
        return tcc;
    }

    const SysFsDriverCPUIntelTcc::Tcc driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelTcc::DRIVER_NAME));

    const uint32_t offset = getData(driver.m_offset).trimmed().toUInt();

    tcc.set_offset(offset);
    tcc.set_max_offset(MAX_OFFSET);

    if(!driver.m_tjMax.empty())
    {
        const uint32_t tjMax = getData(driver.m_tjMax).trimmed().toUInt() / 1000;

        tcc.set_tjmax(tjMax);
        tcc.set_limit(tjMax > offset ? tjMax - offset : 0);

        if(tjMax > MIN_THROTTLING_C)
        {
            tcc.set_max_offset(std::min(MAX_OFFSET,tjMax - MIN_THROTTLING_C));
        }
    }

    return tcc;
}

QByteArray SysFsDataProviderCPUTcc::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    return serializeMessage(read());
}

QByteArray SysFsDataProviderCPUTcc::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::CPUTcc tcc = parseMessage<legion::messages::CPUTcc>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(tcc,Origin::REQUEST);

    if(!tcc.has_offset())
    {
        return {};
    }

    const SysFsDriverCPUIntelTcc::Tcc driver(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelTcc::DRIVER_NAME));

    /*
     * The kernel refuses the write when the offset is locked by the BIOS (the error is sent to the client)
     */
    setData(driver.m_offset,tcc.offset());

    return {};
}

QByteArray SysFsDataProviderCPUTcc::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUTcc>(data,[this,origin](legion::messages::CPUTcc& tcc) { settable(tcc,origin); });
}

void SysFsDataProviderCPUTcc::settable(legion::messages::CPUTcc &tcc, Origin origin) const
{
    tcc.clear_limit();
    tcc.clear_tjmax();
    tcc.clear_max_offset();

    const legion::messages::CPUTcc current = read();

    if(!current.has_max_offset())
    {
        if(tcc.has_offset())
        {
            skipField(origin,"intel TCC offset","not available");
        }

        tcc.Clear();
        return;
    }

    if(tcc.has_offset() && tcc.offset() > current.max_offset())
    {
        skipField(origin,QString("intel TCC offset=%1").arg(tcc.offset()),QString("range <0,%1>").arg(current.max_offset()));
        tcc.clear_offset();
    }
}

}
