// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUOptions.h"
#include <Drivers/SysFsDriverCPUXList.h>



#include <CPUOptions.pb.h>

#include <KitLog.h>
#include <QCoreApplication>
#include <QStringList>

namespace LenovoLegionDaemon::Kit {

SysFsDataProviderCPUOptions::SysFsDataProviderCPUOptions(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderCPUOptions::serializeAndGetData()
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

    settable(cpuOptions,Origin::REQUEST);

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

        if(cpuOptions.cpus().at(i).has_cpu_online())
        {
            setData(cpuXlist.cpuList().at(i).m_cpuOnline.value(),cpuOptions.cpus().at(i).cpu_online());
        }

        if(cpuOptions.cpus().at(i).has_governor())
        {
            setData(cpuXlist.cpuList().at(i).m_freq.m_cpuScalingGovernor,cpuOptions.cpus().at(i).governor());
        }
    }


    m_sysFsDriverManager->processAllUdevEvents(100);
    m_sysFsDriverManager->refreshDriver(SysFsDriverCPUXList::DRIVER_NAME);

    return {};
}

QByteArray SysFsDataProviderCPUOptions::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::CPUOptions>(data,[this,origin](legion::messages::CPUOptions& options) { settable(options,origin); });
}

void SysFsDataProviderCPUOptions::settable(legion::messages::CPUOptions &options, Origin origin) const
{
    SysFsDriverCPUXList::CPUXList cpuXlist(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));

    const int cpuCount = static_cast<int>(cpuXlist.cpuList().size());

    if(options.cpus_size() > cpuCount)
    {
        skipField(origin,QString("CPUs ").append(QString::number(cpuCount)).append("..").append(QString::number(options.cpus_size() - 1)),QString("only ").append(QString::number(cpuCount)).append(" CPUs present"));
        options.mutable_cpus()->DeleteSubrange(cpuCount,options.cpus_size() - cpuCount);
    }

    /*
     * Governors are the same for all CPUs of the scaling driver, the list of an offline CPU can not be read
     */
    std::optional<QStringList> availableGovernors;

    for(const auto& cpu : cpuXlist.cpuList())
    {
        if(const auto governors = getOptionalData(cpu.m_freq.m_cpuScalingAvailableGovernors))
        {
            availableGovernors = governors->split(' ',Qt::SkipEmptyParts);
            break;
        }
    }

    for(int i = 0; i < options.cpus_size(); ++i)
    {
        legion::messages::CPUOptions::CPUX* cpux = options.mutable_cpus(i);
        const auto&                         cpu  = cpuXlist.cpuList().at(i);
        const QString                       name = QString("CPU").append(QString::number(i));

        cpux->clear_available_governors();
        cpux->clear_cpu_core_id();
        cpux->clear_die_id();
        cpux->clear_physical_package_id();
        cpux->clear_cluster_id();

        /*
         * CPU without online attribute (CPU0) is always online
         */
        if(cpux->has_cpu_online() && !cpu.isOnlineAvailable())
        {
            if(!cpux->cpu_online())
            {
                skipField(origin,QString(name).append(" online=0"),"CPU can not be switched off");
            }

            cpux->clear_cpu_online();
        }

        if(!cpux->has_governor())
        {
            continue;
        }

        const QString governor = QString::fromStdString(std::string(cpux->governor())).trimmed();

        /*
         * Governor of a CPU which is switched off by the same message is meaningless (not logged)
         */
        if(governor.isEmpty() || (cpux->has_cpu_online() && !cpux->cpu_online()))
        {
            cpux->clear_governor();
        }
        else if(!cpux->has_cpu_online() && cpu.isOnlineAvailable() && getOptionalData(cpu.m_cpuOnline.value()).value_or("0") != "1")
        {
            skipField(origin,QString(name).append(" governor=").append(governor),"CPU is offline");
            cpux->clear_governor();
        }
        else if(!availableGovernors.has_value() || !availableGovernors->contains(governor))
        {
            skipField(origin,QString(name).append(" governor=").append(governor),QString("available governors: ").append(availableGovernors.value_or(QStringList()).join(' ')));
            cpux->clear_governor();
        }
        else
        {
            cpux->set_governor(governor.toStdString());
        }
    }
}


}
