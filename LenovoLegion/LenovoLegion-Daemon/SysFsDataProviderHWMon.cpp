// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderHWMon.h"
#include "SysFSDriverLegionHWMon.h"
#include "SysFsDriverIntelPowercapRapl.h"
#include "SysFsDriverCPUXList.h"

#include "../LenovoLegion-PrepareBuild/HWMonitoring.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderHWMon::SysFsDataProviderHWMon(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderHWMon::serializeAndGetData() const
{
    legion::messages::HardwareMonitor hardwareMonitoring;
    QByteArray                        byteArray;


    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFSDriverLegionHWMon::HWMon hwMon(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFSDriverLegionHWMon::DRIVER_NAME));

        for(const auto& fanDesc : hwMon.m_legion.m_fans)
        {
            auto fan = hardwareMonitoring.mutable_legion()->add_fans();

            fan->set_fan_label(getData(fanDesc.m_label).toStdString());
            fan->set_fan_speed(getData(fanDesc.m_input).toUInt());
            fan->set_fan_speed_min(getData(fanDesc.m_min).toUInt());
            fan->set_fan_speed_max(getData(fanDesc.m_max).toUInt());
        }

        for(const auto& tempDes : hwMon.m_legion.m_temps)
        {
            auto temp = hardwareMonitoring.mutable_legion()->add_temps();

            temp->set_temp_label(getData(tempDes.m_label).toStdString());
            temp->set_temp_value(getData(tempDes.m_input).toUInt());
        }

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Legion Driver not available");
            hardwareMonitoring.clear_legion();
        }
        else
        {
            throw;
        }
    }


    try {
        SysFsDriverIntelPowercapRapl::IntelPowercapRapl intelPowerapRapl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverIntelPowercapRapl::DRIVER_NAME));

        hardwareMonitoring.mutable_intel_power()->set_power_cap_cpu_energy(getData(intelPowerapRapl.m_powercapCPUEnergy).toULongLong());

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Intel Rapid Driver not available");
             hardwareMonitoring.clear_intel_power();
        }
        else
        {
            throw;
        }
    }

    try {

        SysFsDriverCPUXList::CPUXList cpus(m_sysFsDriverManager->getDriverDescriptorsInVector(SysFsDriverCPUXList::DRIVER_NAME));


        for(size_t i = 0; i < cpus.cpuList().size() ; ++i)
        {
           legion::messages::HardwareMonitor::CPUXFreq* cpFreq =  hardwareMonitoring.add_cpux_freq();



            cpFreq->set_cpu_online(cpus.cpuList().at(i).isOnlineAvailable() ? getData(cpus.cpuList().at(i).m_cpuOnline.value()).toUShort() == 1 : true);


            if(cpFreq->cpu_online())
            {
                cpFreq->set_cpu_base_freq(cpus.cpuList().at(i).m_freq.m_cpuBaseFreq.has_value() ? getData(cpus.cpuList().at(i).m_freq.m_cpuBaseFreq.value()).toUInt() : 0);
                cpFreq->set_cpu_info_min_freq(getData(cpus.cpuList().at(i).m_freq.m_cpuInfoMinFreq).toUInt());
                cpFreq->set_cpu_info_max_freq(getData(cpus.cpuList().at(i).m_freq.m_cpuInfoMaxFreq).toUInt());
                cpFreq->set_cpu_scaling_cur_freq(getData(cpus.cpuList().at(i).m_freq.m_cpuScalingCurFreq).toUInt());
                cpFreq->set_cpu_scaling_min_freq(getData(cpus.cpuList().at(i).m_freq.m_cpuScalingMinFreq).toUInt());
                cpFreq->set_cpu_scaling_max_freq(getData(cpus.cpuList().at(i).m_freq.m_cpuScalingMaxFreq).toUInt());
            }
        }
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- CPUX Driver not available");
            hardwareMonitoring.clear_cpux_freq();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(hardwareMonitoring.ByteSizeLong());
    if(!hardwareMonitoring.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderHWMon::deserializeAndSetData(const QByteArray &)
{
    return {};
}


}
