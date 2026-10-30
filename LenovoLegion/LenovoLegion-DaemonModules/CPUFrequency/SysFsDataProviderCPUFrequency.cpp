// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUFrequency.h"
#include <Drivers/SysFsDriverCPUXList.h>


#include <KitLog.h>
#include <QTimerEvent>

#include <google/protobuf/util/field_mask_util.h>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUFrequency::SysFsDataProviderCPUFrequency(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}

void SysFsDataProviderCPUFrequency::init()
{
    clean();

    if(m_timerId == -1)
    {
        SysFsDriverCPUXList::CPUXList   cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));

        const auto* nestedDesc = legion::messages::CPUFrequency_CPUX::descriptor();

        m_lastCpuFrequency = readCPPFrequency();

        if (nestedDesc != nullptr && !cpuXlist.cpuList().empty()) {
            /*
             * File names are the same for all CPUs, take them from the first one
             */
            const std::string scalingMinFreqName = cpuXlist.cpuList().front().m_freq.m_cpuScalingMinFreq.filename().string();
            const std::string scalingMaxFreqName = cpuXlist.cpuList().front().m_freq.m_cpuScalingMaxFreq.filename().string();

            for (int i = 0; i < nestedDesc->field_count(); ++i) {
                const auto* field = nestedDesc->field(i);
                if (field->name() != scalingMinFreqName && field->name() != scalingMaxFreqName) {
                    m_differencer.IgnoreField(field);
                }
                else
                {
                    LOG_D(QString(__PRETTY_FUNCTION__) + "- Tracking field: " + QString::fromStdString(std::string(field->name())));
                }
            }
        }


        m_timerId = startTimer(std::chrono::milliseconds(500));
    }
}

void SysFsDataProviderCPUFrequency::clean()
{
    if(m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
    }
}


QByteArray SysFsDataProviderCPUFrequency::serializeAndGetData()
{
    QByteArray                             byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    m_lastCpuFrequency = readCPPFrequency();

    byteArray.resize(m_lastCpuFrequency.ByteSizeLong());

    if(!m_lastCpuFrequency.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUFrequency::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverCPUXList::CPUXList   cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));
    legion::messages::CPUFrequency  cpuFrequency;

    LOG_T(__PRETTY_FUNCTION__);


    if(!cpuFrequency.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    settable(cpuFrequency,Origin::REQUEST);

    // Skip if no CPUs in message
    if(cpuFrequency.cpus_size() == 0) {
        LOG_D("SysFsDataProviderCPUFrequency::deserializeAndSetData - no CPUs in message, skipping");
        return {};
    }

    for (int i = 0 ; i < cpuFrequency.cpus_size() ;++i)
    {
        if(i >= static_cast<int>(cpuXlist.cpuList().size()))
        {
            LOG_W(QString(__PRETTY_FUNCTION__) + "- core_id=" + QString::number(i) + QString( " out of range !"));
            break;
        }

        if(cpuFrequency.cpus().at(i).has_scaling_min_freq())
        {
            setData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMinFreq,cpuFrequency.cpus().at(i).scaling_min_freq());
            if(i < m_lastCpuFrequency.cpus_size())
            {
                m_lastCpuFrequency.mutable_cpus(i)->set_scaling_min_freq(cpuFrequency.cpus().at(i).scaling_min_freq());
            }
            else
            {
                LOG_W(QString(__PRETTY_FUNCTION__) + "- core_id=" + QString::number(i) + QString( " out of range in m_lastCpuFrequency !"));
            }
        }

        if(cpuFrequency.cpus().at(i).has_scaling_max_freq())
        {
            setData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMaxFreq,cpuFrequency.cpus().at(i).scaling_max_freq());
            if(i < m_lastCpuFrequency.cpus_size())
            {
                m_lastCpuFrequency.mutable_cpus(i)->set_scaling_max_freq(cpuFrequency.cpus().at(i).scaling_max_freq());
            }
            else
            {
                LOG_W(QString(__PRETTY_FUNCTION__) + "- core_id=" + QString::number(i) + QString( " out of range in m_lastCpuFrequency !"));
            }
        }
    }

    return {};
}

QByteArray SysFsDataProviderCPUFrequency::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUFrequency>(data,[this,origin](legion::messages::CPUFrequency& frequency) { settable(frequency,origin); });
}

void SysFsDataProviderCPUFrequency::settable(legion::messages::CPUFrequency &frequency, Origin origin) const
{
    SysFsDriverCPUXList::CPUXList cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));

    const int cpuCount = static_cast<int>(cpuXlist.cpuList().size());

    if(frequency.cpus_size() > cpuCount)
    {
        skipField(origin,QString("CPUs ").append(QString::number(cpuCount)).append("..").append(QString::number(frequency.cpus_size() - 1)),QString("only ").append(QString::number(cpuCount)).append(" CPUs present"));
        frequency.mutable_cpus()->DeleteSubrange(cpuCount,frequency.cpus_size() - cpuCount);
    }

    for(int i = 0; i < frequency.cpus_size(); ++i)
    {
        legion::messages::CPUFrequency::CPUX* cpux = frequency.mutable_cpus(i);
        const auto&                           cpu  = cpuXlist.cpuList().at(i);
        const QString                         name = QString("CPU").append(QString::number(i));

        cpux->clear_base_freq();
        cpux->clear_min_freq();
        cpux->clear_max_freq();
        cpux->clear_scaling_cur_freq();
        cpux->clear_online();
        cpux->clear_core_id();
        cpux->clear_die_id();
        cpux->clear_physical_package_id();
        cpux->clear_cluster_id();

        if(!cpux->has_scaling_min_freq() && !cpux->has_scaling_max_freq())
        {
            continue;
        }

        /*
         * Limits of the hardware, not readable when the CPU is offline
         */
        const auto minFreq = getOptionalData(cpu.m_freq.m_cpuInfoMinFreq);
        const auto maxFreq = getOptionalData(cpu.m_freq.m_cpuInfoMaxFreq);

        if(!minFreq.has_value() || !maxFreq.has_value())
        {
            skipField(origin,QString(name).append(" scaling limits"),"CPU is offline");
            cpux->clear_scaling_min_freq();
            cpux->clear_scaling_max_freq();
            continue;
        }

        const uint min = minFreq->toUInt();
        const uint max = maxFreq->toUInt();

        auto inRange = [&](bool has,uint value,const char* field) {
            if(has && (value < min || value > max))
            {
                skipField(origin,QString(name).append(' ').append(field).append('=').append(QString::number(value)),QString("range <").append(QString::number(min)).append(',').append(QString::number(max)).append('>'));
                return false;
            }
            return true;
        };

        if(!inRange(cpux->has_scaling_min_freq(),cpux->scaling_min_freq(),"scaling_min_freq"))
        {
            cpux->clear_scaling_min_freq();
        }

        if(!inRange(cpux->has_scaling_max_freq(),cpux->scaling_max_freq(),"scaling_max_freq"))
        {
            cpux->clear_scaling_max_freq();
        }
    }
}

legion::messages::CPUFrequency SysFsDataProviderCPUFrequency::readCPPFrequency(bool scalingLimitsOnly) const
{
    legion::messages::CPUFrequency         cpuFrequency;

    try {
        SysFsDriverCPUXList::CPUXList cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));

        for(size_t i = 0; i < cpuXlist.cpuList().size() ; ++i)
        {
            legion::messages::CPUFrequency::CPUX *cpux = cpuFrequency.add_cpus();

            cpux->set_online(cpuXlist.cpuList().at(i).isOnlineAvailable() ? getData(cpuXlist.cpuList().at(i).m_cpuOnline.value()).toUShort() == 1 : true);


            if(cpux->online())
            {
                if(!cpuXlist.cpuList().at(i).m_topology.has_value())
                {
                    LOG_D(QString(__PRETTY_FUNCTION__) + "- CPUX Driver Topology not available");
                    break;
                }

                cpux->set_scaling_min_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMinFreq).toUInt());
                cpux->set_scaling_max_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMaxFreq).toUInt());

                if(scalingLimitsOnly)
                {
                    continue;
                }

                if(cpuXlist.cpuList().at(i).m_freq.m_cpuBaseFreq.has_value())
                {
                    cpux->set_base_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuBaseFreq.value()).toUInt());
                }

                cpux->set_min_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuInfoMinFreq).toUInt());
                cpux->set_max_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuInfoMaxFreq).toUInt());
                cpux->set_scaling_cur_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingCurFreq).toUInt());

                cpux->set_core_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_coreId).toUInt());
                cpux->set_die_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_dieId).toUInt());
                cpux->set_physical_package_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_physicalPackageId).toUInt());
                cpux->set_cluster_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_clusterId).toUInt());
            }
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuFrequency.Clear();
        }
        else
        {
            throw;
        }
    }


    return cpuFrequency;
}

void SysFsDataProviderCPUFrequency::timerEvent(QTimerEvent *event)
{
    if(event->timerId() == m_timerId)
    {
        // Differencer tracks only scaling limits, other fields are not read
        legion::messages::CPUFrequency cpuFrequency = readCPPFrequency(true);

        if (!m_differencer.Compare(cpuFrequency, m_lastCpuFrequency))
        {
            m_lastCpuFrequency = cpuFrequency;

            LOG_D(QString(__PRETTY_FUNCTION__) + "- CPU Frequency changed, emitting dataChanged signal");

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