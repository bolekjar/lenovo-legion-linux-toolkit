// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUTopology.h"
#include "SysFsDriverCPUCore.h"
#include "SysFsDriverCPUAtom.h"
#include "SysFsDriverCPU.h"


#include "../LenovoLegion-PrepareBuild/CPUTopology.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderCPUTopology::SysFsDataProviderCPUTopology(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderCPUTopology::serializeAndGetData() const
{
    legion::messages::CPUTopology     cpuTopologyData;
    QByteArray                        byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverCPUCore::CPUCore core(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUCore::DRIVER_NAME));
        SysFsDriverCPUAtom::CPUAtom atom(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPUAtom::DRIVER_NAME));
        SysFsDriverCPU::CPU cpu(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));

        if(!getData(core.m_cpus).trimmed().isEmpty())
        {
            auto cpusCoreTopology = getData(core.m_cpus).split(',');
            for (unsigned int i = 0; i < cpusCoreTopology.size(); ++i) {
                auto range = cpusCoreTopology.at(i).split('-');
                legion::messages::CPUTopology::ActiveCPUsRange* activeRange = cpuTopologyData.add_active_cpus_core();

                if(range.size() == 1)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(0).toUShort());
                }
                else if(range.size() == 2)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(1).toUShort());
                }
                else
                {
                    THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU topology data !").c_str());
                }
            }
        }

        if(!getData(atom.m_cpus).trimmed().isEmpty())
        {
            auto cpusAtomTopology = getData(atom.m_cpus).split(',');
            for (unsigned int i = 0; i < cpusAtomTopology.size(); ++i) {
                auto range = cpusAtomTopology.at(i).split('-');
                legion::messages::CPUTopology::ActiveCPUsRange* activeRange = cpuTopologyData.add_active_cpus_atom();

                if(range.size() == 1)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(0).toUShort());
                }
                else if(range.size() == 2)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(1).toUShort());
                }
                else
                {
                    THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU topology data !").c_str());
                }
            }
        }

        if(!getData(cpu.m_topology.m_online).trimmed().isEmpty())
        {
            auto cpusTopology = getData(cpu.m_topology.m_online).split(',');
            for (unsigned int i = 0; i <cpusTopology.size(); ++i) {
                auto range = cpusTopology.at(i).split('-');
                legion::messages::CPUTopology::ActiveCPUsRange* activeRange = cpuTopologyData.add_active_cpus();

                if(range.size() == 1)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(0).toUShort());
                }
                else if(range.size() == 2)
                {
                    activeRange->set_cpu_min(range.at(0).toUShort());
                    activeRange->set_cpu_max(range.at(1).toUShort());
                }
                else
                {
                    THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU topology data !").c_str());
                }
            }
        }

        if(!getData(cpu.m_topology.m_present).trimmed().isEmpty())
        {
            auto cpusTopology = getData(cpu.m_topology.m_possible).split(',');

            for (unsigned int i = 0; i <cpusTopology.size(); ++i) {
                auto range = cpusTopology.at(i).split('-');
                if(range.size() == 1)
                {
                    cpuTopologyData.mutable_possible_cpus()->set_cpu_min(std::min(cpuTopologyData.possible_cpus().cpu_min(),range.at(0).toUInt()));
                    cpuTopologyData.mutable_possible_cpus()->set_cpu_max(std::max(cpuTopologyData.possible_cpus().cpu_max(),range.at(0).toUInt()));
                }
                else if(range.size() == 2)
                {
                    cpuTopologyData.mutable_possible_cpus()->set_cpu_min(std::min(cpuTopologyData.possible_cpus().cpu_min(),range.at(0).toUInt()));
                    cpuTopologyData.mutable_possible_cpus()->set_cpu_max(std::max(cpuTopologyData.possible_cpus().cpu_max(),range.at(1).toUInt()));
                }
                else
                {
                    THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU topology data !").c_str());
                }
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
