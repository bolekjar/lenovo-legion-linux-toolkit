// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDataProviderCPUPState.h"
#include <Drivers/SysFsDriverCPUIntelPState.h>

#include <KitLog.h>

#include <QTimerEvent>

namespace LenovoLegionDaemon::Kit {


SysFsDataProviderCPUPState::SysFsDataProviderCPUPState(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

QByteArray SysFsDataProviderCPUPState::serializeAndGetData()
{
    QByteArray                     byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    legion::messages::CPUPState    cpuPState = readCPPPState();

    byteArray.resize(cpuPState.ByteSizeLong());
    if(!cpuPState.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUPState::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::CPUPState cpuPState;

    LOG_T(__PRETTY_FUNCTION__);

    if(!cpuPState.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(cpuPState,Origin::REQUEST);

    {
        SysFsDriverCPUIntelPState::IntelPState intePState(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelPState::DRIVER_NAME));

        if(cpuPState.has_status() && intePState.m_status.has_value()) {
            //The driver change his structure need to block kernel event to avoid race condition
            m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUIntelPState::DRIVER_NAME,true);
            auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUIntelPState::DRIVER_NAME,false); });

            setData(intePState.m_status.value(),cpuPState.status());
            m_lastCpuPState.set_status(cpuPState.status());

            m_sysFsDriverManager->processAllUdevEvents(100);
            m_sysFsDriverManager->refreshDriver(SysFsDriverCPUIntelPState::DRIVER_NAME);
        }
    }

    SysFsDriverCPUIntelPState::IntelPState intePState(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelPState::DRIVER_NAME));


    if(cpuPState.has_hwp_dynamic_boost() && intePState.m_hwp_dynamic_boost.has_value()) {
        setData(intePState.m_hwp_dynamic_boost.value(),cpuPState.hwp_dynamic_boost());
        m_lastCpuPState.set_hwp_dynamic_boost(cpuPState.hwp_dynamic_boost());
    }
    if(cpuPState.has_no_turbo() && intePState.m_no_turbo.has_value()) {
        setData(intePState.m_no_turbo.value(),cpuPState.no_turbo());
        m_lastCpuPState.set_no_turbo(cpuPState.no_turbo());
    }


    //
    // You have to set twice becase only one can be set due to min < max. You do not know what is set.
    // So you have to set twice to make sure both are set.
    //
    if(cpuPState.has_max_perf_pct() && intePState.m_max_perf_pct.has_value()) {
        setData(intePState.m_max_perf_pct.value(),cpuPState.max_perf_pct());
        m_lastCpuPState.set_max_perf_pct(cpuPState.max_perf_pct());
    }
    if(cpuPState.has_min_perf_pct() && intePState.m_min_perf_pct.has_value()) {
        setData(intePState.m_min_perf_pct.value(),cpuPState.min_perf_pct());
        m_lastCpuPState.set_min_perf_pct(cpuPState.min_perf_pct());
    }
    if(cpuPState.has_max_perf_pct() && intePState.m_max_perf_pct.has_value()) {
        setData(intePState.m_max_perf_pct.value(),cpuPState.max_perf_pct());
        m_lastCpuPState.set_max_perf_pct(cpuPState.max_perf_pct());
    }
    if(cpuPState.has_min_perf_pct() && intePState.m_min_perf_pct.has_value()) {
        setData(intePState.m_min_perf_pct.value(),cpuPState.min_perf_pct());
        m_lastCpuPState.set_min_perf_pct(cpuPState.min_perf_pct());
    }

    return {};
}

void SysFsDataProviderCPUPState::init()
{
    clean();

    if(m_timerId == -1)
    {
        m_lastCpuPState = readCPPPState();
        m_timerId = startTimer(std::chrono::milliseconds(500));
    }
}

void SysFsDataProviderCPUPState::clean()
{
    if(m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
    }
}

QByteArray SysFsDataProviderCPUPState::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUPState>(data,[this,origin](legion::messages::CPUPState& pstate) { settable(pstate,origin); });
}

void SysFsDataProviderCPUPState::settable(legion::messages::CPUPState &pstate, Origin origin) const
{
    /*
     * Presence of the other attributes depends on the status, it is checked when they are written (after the status is set)
     */
    if(pstate.has_status())
    {
        const QString status = QString::fromStdString(std::string(pstate.status())).trimmed();

        /*
         * "off" unloads the scaling driver, it is never set by the daemon
         */
        if(status != "active" && status != "passive")
        {
            skipField(origin,QString("intel_pstate status=").append(status),"only active/passive can be set");
            pstate.clear_status();
        }
        else
        {
            pstate.set_status(status.toStdString());
        }
    }

    if(pstate.has_min_perf_pct() && pstate.min_perf_pct() > 100)
    {
        skipField(origin,QString("intel_pstate min_perf_pct=").append(QString::number(pstate.min_perf_pct())),"range <0,100>");
        pstate.clear_min_perf_pct();
    }

    if(pstate.has_max_perf_pct() && pstate.max_perf_pct() > 100)
    {
        skipField(origin,QString("intel_pstate max_perf_pct=").append(QString::number(pstate.max_perf_pct())),"range <0,100>");
        pstate.clear_max_perf_pct();
    }
}

legion::messages::CPUPState SysFsDataProviderCPUPState::readCPPPState() const
{
    legion::messages::CPUPState    cpuPState;

    try {
        SysFsDriverCPUIntelPState::IntelPState intePState(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelPState::DRIVER_NAME));

        if(intePState.m_hwp_dynamic_boost.has_value())
        {
            cpuPState.set_hwp_dynamic_boost(getData(intePState.m_hwp_dynamic_boost.value()).toUShort() == 1);
        }

        if(intePState.m_min_perf_pct.has_value())
        {
            cpuPState.set_min_perf_pct(getData(intePState.m_min_perf_pct.value()).toUInt());
        }

        if(intePState.m_max_perf_pct.has_value())
        {
            cpuPState.set_max_perf_pct(getData(intePState.m_max_perf_pct.value()).toUInt());
        }

        if(intePState.m_no_turbo.has_value())
        {
            cpuPState.set_no_turbo(getData(intePState.m_no_turbo.value()).toShort() == 1);
        }

        if (intePState.m_status.has_value())
        {
            cpuPState.set_status(getData(intePState.m_status.value()).toStdString());
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuPState.Clear();
        }
        else
        {
            throw;
        }
    }

    return cpuPState;
}


void SysFsDataProviderCPUPState::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timerId)
    {
        SysFsDriverCPUIntelPState::IntelPState intePState(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUIntelPState::DRIVER_NAME));

        std::string curStatus;
        if (intePState.m_status.has_value())
        {
            curStatus = getData(intePState.m_status.value()).toStdString();

            if(curStatus != m_lastCpuPState.status())
            {
                LOG_D(QString(__PRETTY_FUNCTION__) + "- CPU PState status changed from " + QString::fromStdString(std::string(m_lastCpuPState.status())) + " to " + QString::fromStdString(std::string(curStatus)));
                m_sysFsDriverManager->refreshDriver(SysFsDriverCPUIntelPState::DRIVER_NAME);
            }
        }

        legion::messages::CPUPState cpuPstate = readCPPPState();

        if (!m_differencer.Compare(cpuPstate, m_lastCpuPState))
        {
            m_lastCpuPState = cpuPstate;

            LOG_D(QString(__PRETTY_FUNCTION__) + "- CPU PState changed, emitting dataChanged signal");

            emit dataProviderEvent(DataProvider::SubsystemEvent {
                .m_dataType = this->m_dataType,
                .m_action = DataProvider::SubsystemEvent::Action::CHANGED,
                .m_DataProviderSpecificEventType    = {},
                .m_DataProviderSpecificEventValue   = {}
            });
        }
    }
}



}