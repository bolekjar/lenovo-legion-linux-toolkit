// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUFrequency.h"
#include "SysFsDriverCPUXList.h"


#include "../LenovoLegion-PrepareBuild/CPUFrequency.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderCPUFrequency::SysFsDataProviderCPUFrequency(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderCPUFrequency::serializeAndGetData() const
{
    legion::messages::CPUFrequency         cpuFrequency;
    QByteArray                             byteArray;

    LOG_T(__PRETTY_FUNCTION__);

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

                if(cpuXlist.cpuList().at(i).m_freq.m_cpuBaseFreq.has_value())
                {
                    cpux->set_base_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuBaseFreq.value()).toUInt());
                }

                cpux->set_min_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuInfoMinFreq).toUInt());
                cpux->set_max_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuInfoMaxFreq).toUInt());
                cpux->set_scaling_cur_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingCurFreq).toUInt());
                cpux->set_scaling_min_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMinFreq).toUInt());
                cpux->set_scaling_max_freq(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMaxFreq).toUInt());


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
    byteArray.resize(cpuFrequency.ByteSizeLong());
    if(!cpuFrequency.SerializeToArray(byteArray.data(),byteArray.size()))
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
        }

        if(cpuFrequency.cpus().at(i).has_scaling_max_freq())
        {
            setData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingMaxFreq,cpuFrequency.cpus().at(i).scaling_max_freq());
        }
    }

    return {};
}


}
