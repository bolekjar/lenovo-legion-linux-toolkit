// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUTopology.h"
#include <Drivers/SysFsDriverCPUCore.h>
#include <Drivers/SysFsDriverCPUAtom.h>
#include <Drivers/SysFsDriverCPU.h>


#include <CPUTopology.pb.h>


#include <KitLog.h>


namespace LenovoLegionDaemon::Kit {

namespace {

/*
 * CPU list of the sysfs ("0-3,8,10-11") as the ranges of the message, an empty list adds nothing
 */
void addRanges(const QString& list, google::protobuf::RepeatedPtrField<legion::messages::CPUTopology::ActiveCPUsRange>* ranges)
{
    if(list.trimmed().isEmpty())
    {
        return;
    }

    for (const auto& item : list.trimmed().split(',')) {
        const auto range = item.split('-');

        if(range.size() != 1 && range.size() != 2)
        {
            THROW_EXCEPTION(SysFsDataProvider::exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU topology data !").c_str());
        }

        legion::messages::CPUTopology::ActiveCPUsRange* activeRange = ranges->Add();

        activeRange->set_cpu_min(range.front().toUInt());
        activeRange->set_cpu_max(range.back().toUInt());
    }
}

}

SysFsDataProviderCPUTopology::SysFsDataProviderCPUTopology(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderCPUTopology::serializeAndGetData()
{
    legion::messages::CPUTopology     cpuTopologyData;
    QByteArray                        byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverCPU::CPU cpu(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));

        /*
         * P- and E-cores only on a hybrid Intel CPU (/sys/devices/cpu_core, cpu_atom), not on AMD or older Intel CPUs
         */
        if(m_sysFsDriverManager->isLoaded(SysFsDriverCPUCore::DRIVER_NAME))
        {
            SysFsDriverCPUCore::CPUCore core(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUCore::DRIVER_NAME));

            addRanges(getData(core.m_cpus),cpuTopologyData.mutable_active_cpus_core());
        }

        if(m_sysFsDriverManager->isLoaded(SysFsDriverCPUAtom::DRIVER_NAME))
        {
            SysFsDriverCPUAtom::CPUAtom atom(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAtom::DRIVER_NAME));

            addRanges(getData(atom.m_cpus),cpuTopologyData.mutable_active_cpus_atom());
        }

        addRanges(getData(cpu.m_topology.m_online),cpuTopologyData.mutable_active_cpus());

        /*
         * Possible CPUs: from the first to the last one
         */
        google::protobuf::RepeatedPtrField<legion::messages::CPUTopology::ActiveCPUsRange> possible;

        addRanges(getData(cpu.m_topology.m_possible),&possible);

        if(!possible.empty())
        {
            cpuTopologyData.mutable_possible_cpus()->set_cpu_min(possible.Get(0).cpu_min());

            for (const auto& range : possible) {
                cpuTopologyData.mutable_possible_cpus()->set_cpu_min(std::min(cpuTopologyData.possible_cpus().cpu_min(),range.cpu_min()));
                cpuTopologyData.mutable_possible_cpus()->set_cpu_max(std::max(cpuTopologyData.possible_cpus().cpu_max(),range.cpu_max()));
            }
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuTopologyData.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(cpuTopologyData.ByteSizeLong());
    if(!cpuTopologyData.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUTopology::deserializeAndSetData(const QByteArray &)
{
    return {};
}


}
