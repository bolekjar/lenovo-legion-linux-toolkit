// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUAmdPState.h"

#include <Drivers/SysFsDriverCPUAmdPState.h>

#include <KitLog.h>

#include <QScopeGuard>
#include <QTimerEvent>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUAmdPState::SysFsDataProviderCPUAmdPState(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

QByteArray SysFsDataProviderCPUAmdPState::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    return serializeMessage(read());
}

QByteArray SysFsDataProviderCPUAmdPState::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::CPUAmdPState pstate = parseMessage<legion::messages::CPUAmdPState>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(pstate,Origin::REQUEST);

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUAmdPState::DRIVER_NAME))
    {
        return {};
    }

    /*
     * Mode first: it recreates the cpufreq policies (EPP exists only in the active mode)
     */
    if(pstate.has_status())
    {
        const SysFsDriverCPUAmdPState::AmdPState amd(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAmdPState::DRIVER_NAME));

        m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUAmdPState::DRIVER_NAME,true);
        auto cleanup = qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUAmdPState::DRIVER_NAME,false); });

        if(getData(amd.m_status).trimmed().toStdString() != pstate.status())
        {
            setData(amd.m_status,std::string_view(pstate.status()));
            m_sysFsDriverManager->processAllUdevEvents(100);
        }

        m_sysFsDriverManager->refreshDriver(SysFsDriverCPUAmdPState::DRIVER_NAME);
    }

    const SysFsDriverCPUAmdPState::AmdPState amd(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAmdPState::DRIVER_NAME));

    if(pstate.has_boost())
    {
        for(const auto& path : amd.m_boost)
        {
            setData(path,pstate.boost());
        }
    }

    /*
     * EPP after the mode: checked again against the values of the (new) mode
     */
    if(pstate.has_energy_performance_preference())
    {
        const QStringList available = amd.m_eppAvailable.empty() ? QStringList() : getData(amd.m_eppAvailable).split(' ',Qt::SkipEmptyParts);
        const QString     epp       = QString::fromStdString(std::string(pstate.energy_performance_preference()));

        if(available.contains(epp))
        {
            for(const auto& path : amd.m_epp)
            {
                setData(path,std::string_view(pstate.energy_performance_preference()));
            }
        }
        else
        {
            skipField(Origin::REQUEST,QString("amd_pstate energy_performance_preference=").append(epp),
                      QString("available: ").append(available.join(',')));
        }
    }

    m_last = read();

    return {};
}

void SysFsDataProviderCPUAmdPState::init()
{
    clean();

    if(m_timerId == -1)
    {
        m_last    = read();
        m_timerId = startTimer(std::chrono::milliseconds(500));
    }
}

void SysFsDataProviderCPUAmdPState::clean()
{
    if(m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
    }
}

QByteArray SysFsDataProviderCPUAmdPState::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUAmdPState>(data,[this,origin](legion::messages::CPUAmdPState& pstate) { settable(pstate,origin); });
}

void SysFsDataProviderCPUAmdPState::settable(legion::messages::CPUAmdPState &pstate, Origin origin) const
{
    /*
     * Read-only values; the EPP is checked when it is written (its values depend on the mode set before it)
     */
    pstate.clear_settable_statuses();
    pstate.clear_available_energy_performance_preferences();
    pstate.clear_prefcore();

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUAmdPState::DRIVER_NAME))
    {
        if(pstate.has_status() || pstate.has_boost() || pstate.has_energy_performance_preference())
        {
            skipField(origin,"amd_pstate","driver not available");
        }

        pstate.Clear();
        return;
    }

    if(pstate.has_status())
    {
        const QString status = QString::fromStdString(std::string(pstate.status())).trimmed();

        if(!SETTABLE_STATUSES.contains(status))
        {
            skipField(origin,QString("amd_pstate status=").append(status),QString("settable: ").append(SETTABLE_STATUSES.join(',')));
            pstate.clear_status();
        }
        else
        {
            pstate.set_status(status.toStdString());
        }
    }

    if(pstate.has_boost() && SysFsDriverCPUAmdPState::AmdPState(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAmdPState::DRIVER_NAME)).m_boost.empty())
    {
        skipField(origin,"amd_pstate boost","not available");
        pstate.clear_boost();
    }
}

legion::messages::CPUAmdPState SysFsDataProviderCPUAmdPState::read() const
{
    legion::messages::CPUAmdPState pstate;

    if(!m_sysFsDriverManager->isLoaded(SysFsDriverCPUAmdPState::DRIVER_NAME))
    {
        return pstate;
    }

    const SysFsDriverCPUAmdPState::AmdPState amd(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAmdPState::DRIVER_NAME));

    pstate.set_status(getData(amd.m_status).trimmed().toStdString());

    for(const auto& status : SETTABLE_STATUSES)
    {
        pstate.add_settable_statuses(status.toStdString());
    }

    if(!amd.m_boost.empty())
    {
        pstate.set_boost(getData(amd.m_boost.front()).trimmed() == "1");
    }

    if(!amd.m_epp.empty())
    {
        pstate.set_energy_performance_preference(getData(amd.m_epp.front()).trimmed().toStdString());
    }

    if(!amd.m_eppAvailable.empty())
    {
        for(const auto& epp : getData(amd.m_eppAvailable).split(' ',Qt::SkipEmptyParts))
        {
            pstate.add_available_energy_performance_preferences(epp.trimmed().toStdString());
        }
    }

    if(!amd.m_prefcore.empty())
    {
        pstate.set_prefcore(getData(amd.m_prefcore).trimmed().toStdString());
    }

    return pstate;
}

void SysFsDataProviderCPUAmdPState::timerEvent(QTimerEvent *event)
{
    if(event->timerId() != m_timerId || !m_sysFsDriverManager->isLoaded(SysFsDriverCPUAmdPState::DRIVER_NAME))
    {
        return;
    }

    /*
     * The mode changed by another tool recreates the policies (EPP files)
     */
    const SysFsDriverCPUAmdPState::AmdPState amd(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAmdPState::DRIVER_NAME));

    if(getData(amd.m_status).trimmed().toStdString() != m_last.status())
    {
        LOG_D(QString(__PRETTY_FUNCTION__) + "- AMD P-State status changed to " + getData(amd.m_status).trimmed());
        m_sysFsDriverManager->refreshDriver(SysFsDriverCPUAmdPState::DRIVER_NAME);
    }

    const legion::messages::CPUAmdPState current = read();

    if(!m_differencer.Compare(current,m_last))
    {
        m_last = current;

        LOG_D(QString(__PRETTY_FUNCTION__) + "- AMD P-State changed, emitting dataChanged signal");

        emit dataProviderEvent(DataProvider::SubsystemEvent {
            .m_dataType = this->m_dataType,
            .m_action = DataProvider::SubsystemEvent::Action::CHANGED,
            .m_DataProviderSpecificEventType    = {},
            .m_DataProviderSpecificEventValue   = {}
        });
    }
}

}
