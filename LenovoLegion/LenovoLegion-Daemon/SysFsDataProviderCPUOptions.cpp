// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUOptions.h"
#include "SysFsDriverCPUXList.h"



#include "../LenovoLegion-PrepareBuild/CPUOptions.pb.h"

#include <Core/LoggerHolder.h>
#include <QCoreApplication>

namespace LenovoLegionDaemon {

SysFsDataProviderCPUOptions::SysFsDataProviderCPUOptions(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderCPUOptions::serializeAndGetData() const
{
    legion::messages::CPUOptions       cpuOption;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverCPUXList::CPUXList cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));

        for(size_t i = 0; i < cpuXlist.cpuList().size() ; ++i)
        {
            legion::messages::CPUOptions::CPUX *cpux = cpuOption.add_cpus();

            cpux->set_cpu_online(cpuXlist.cpuList().at(i).isOnlineAvailable() ? getData(cpuXlist.cpuList().at(i).m_cpuOnline.value()).toUShort() == 1 : true);

            if(cpux->cpu_online())
            {

                if(!cpuXlist.cpuList().at(i).m_topology.has_value())
                {
                    LOG_D(QString(__PRETTY_FUNCTION__) + "- CPUX Driver Topology not available");
                    break;
                }

                cpux->set_available_governors(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingAvailableGovernors).toStdString());
                cpux->set_governor(getData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingGovernor).toStdString());

                cpux->set_cpu_core_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_coreId).toUInt());
                cpux->set_die_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_dieId).toUInt());
                cpux->set_cluster_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_clusterId).toUInt());
                cpux->set_physical_package_id(getData(cpuXlist.cpuList().at(i).m_topology.value().m_physicalPackageId).toUInt());
            }
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuOption.Clear();
        }
        else
        {
            throw;
        }
    }


    byteArray.resize(cpuOption.ByteSizeLong());
    if(!cpuOption.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUOptions::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverCPUXList::CPUXList cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));
    legion::messages::CPUOptions  cpuOptions;

    LOG_T(__PRETTY_FUNCTION__);

    if(!cpuOptions.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    // Skip if no CPUs in message
    if(cpuOptions.cpus_size() == 0) {
        LOG_D("SysFsDataProviderCPUOptions::deserializeAndSetData - no CPUs in message, skipping");
        return {};
    }

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,false); });



    for (int i = 0 ; i < cpuOptions.cpus_size() ;++i)
    {
        if(i >= static_cast<int>(cpuXlist.cpuList().size()))
        {
            LOG_W(QString(__PRETTY_FUNCTION__) + "- core_id=" + QString::number(i) + QString( " out of range !"));
            break;
        }

        if(cpuXlist.cpuList().at(i).isOnlineAvailable())
        {
            setData(cpuXlist.cpuList().at(i).m_cpuOnline.value(),cpuOptions.cpus().at(i).cpu_online());
        }

        if(!cpuOptions.cpus().at(i).governor().empty())
        {
            setData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingGovernor,cpuOptions.cpus().at(i).governor());
        }
    }


    m_sysFsDriverManager->processAllUdevEvents(100);
    m_sysFsDriverManager->refreshDriver(SysFsDriverCPUXList::DRIVER_NAME);

    return {};
}


}
