// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUPower.h"
#include "SysFsDriverLegionOther.h"

#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderCPUPower::SysFsDataProviderCPUPower(SysFsDriverManager* sysFsDriverManager, QObject* parent) :
    SysFsDataProvider(sysFsDriverManager,parent,dataType)
{}



QByteArray SysFsDataProviderCPUPower::serializeAndGetData() const
{
    legion::messages::CPUPower         cpuPower;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFsDriverLegionOther::Other::CPU cpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
        SysFsDriverLegionOther::Other::GPU gpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        /*
         * STP power limit
         */
        cpuPower.mutable_cpu_stp_limit()->set_current_value(static_cast<quint8>(getData(cpuControl.m_cpu_stp_limit.m_current_value).toUShort()));
        setValue(cpuControl.m_cpu_stp_limit.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_cpu_stp_limit()->mutable_mode_descriptor_map());

        setValue(cpuControl.m_cpu_stp_limit.m_max_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },cpuPower.mutable_cpu_stp_limit()->mutable_mode_descriptor_map());

        setValue(cpuControl.m_cpu_stp_limit.m_min_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },cpuPower.mutable_cpu_stp_limit()->mutable_mode_descriptor_map());

        setValue(cpuControl.m_cpu_stp_limit.m_scalar_increment,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },cpuPower.mutable_cpu_stp_limit()->mutable_mode_descriptor_map());

        setValue(cpuControl.m_cpu_stp_limit.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_cpu_stp_limit()->mutable_mode_descriptor_map());


        /*
         * LTP power limit
         */
        cpuPower.mutable_cpu_ltp_limit()->set_current_value(static_cast<quint8>(getData(cpuControl.m_cpu_ltp_limit.m_current_value).toUShort()));
        setValue(cpuControl.m_cpu_ltp_limit.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_cpu_ltp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_ltp_limit.m_max_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },cpuPower.mutable_cpu_ltp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_ltp_limit.m_min_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },cpuPower.mutable_cpu_ltp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_ltp_limit.m_scalar_increment,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },cpuPower.mutable_cpu_ltp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_ltp_limit.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_cpu_ltp_limit()->mutable_mode_descriptor_map());



        /*
         * CLP power limit
         */
        cpuPower.mutable_cpu_clp_limit()->set_current_value(static_cast<quint8>(getData(cpuControl.m_cpu_clp_limit.m_current_value).toUShort()));
        setValue(cpuControl.m_cpu_clp_limit.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_cpu_clp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_clp_limit.m_max_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },cpuPower.mutable_cpu_clp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_clp_limit.m_min_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },cpuPower.mutable_cpu_clp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_clp_limit.m_scalar_increment,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },cpuPower.mutable_cpu_clp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_clp_limit.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_cpu_clp_limit()->mutable_mode_descriptor_map());



        /*
         * TMP power limit
         */
        cpuPower.mutable_cpu_tmp_limit()->set_current_value(static_cast<quint8>(getData(cpuControl.m_cpu_tmp_limit.m_current_value).toUShort()));
        setValue(cpuControl.m_cpu_tmp_limit.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_cpu_tmp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_tmp_limit.m_max_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },cpuPower.mutable_cpu_tmp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_tmp_limit.m_min_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },cpuPower.mutable_cpu_tmp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_tmp_limit.m_scalar_increment,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },cpuPower.mutable_cpu_tmp_limit()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_tmp_limit.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_cpu_tmp_limit()->mutable_mode_descriptor_map());


        /*
         * PL1 tau
         */
        cpuPower.mutable_cpu_pl1_tau()->set_current_value(static_cast<quint8>(getData(cpuControl.m_cpu_pl1_tau.m_current_value).toUShort()));
        setValue(cpuControl.m_cpu_pl1_tau.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_cpu_pl1_tau()->mutable_mode_descriptor_map());
        setValue(cpuControl.m_cpu_pl1_tau.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_cpu_pl1_tau()->mutable_mode_descriptor_map());
        setValuesSteps(cpuControl.m_cpu_pl1_tau.m_steps,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,const QList<QString>& values){
            for (const auto& val : values) {
                descriptor.add_steps(val.toUInt());
            }
        },cpuPower.mutable_cpu_pl1_tau()->mutable_mode_descriptor_map());


        /*
         * GPU total on ac
         */
        cpuPower.mutable_gpu_total_onac()->set_current_value(static_cast<quint8>(getData(gpuControl.m_gpu_total_onac.m_current_value).toUShort()));
        setValue(gpuControl.m_gpu_total_onac.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_gpu_total_onac()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_total_onac.m_max_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },cpuPower.mutable_gpu_total_onac()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_total_onac.m_min_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },cpuPower.mutable_gpu_total_onac()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_total_onac.m_scalar_increment,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },cpuPower.mutable_gpu_total_onac()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_total_onac.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_gpu_total_onac()->mutable_mode_descriptor_map());


        /*
         * GPU to CPU dynamic boost
         */
        cpuPower.mutable_gpu_to_cpu_dynamic_boost()->set_current_value(static_cast<quint8>(getData(gpuControl.m_gpu_to_cpu_dynamic_boost.m_current_value).toUShort()));
        setValue(gpuControl.m_gpu_to_cpu_dynamic_boost.m_default_value,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },cpuPower.mutable_gpu_to_cpu_dynamic_boost()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_to_cpu_dynamic_boost.m_supported,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },cpuPower.mutable_gpu_to_cpu_dynamic_boost()->mutable_mode_descriptor_map());
        setValuesSteps(gpuControl.m_gpu_to_cpu_dynamic_boost.m_steps,[&](legion::messages::CPUPower::Limit::Descriptor &descriptor,const QList<QString>& values){
            for (const auto& val : values) {
                descriptor.add_steps(val.toUInt());
            }
        },cpuPower.mutable_gpu_to_cpu_dynamic_boost()->mutable_mode_descriptor_map());

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuPower.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(cpuPower.ByteSizeLong());
    if(!cpuPower.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUPower::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverLegionOther::Other::CPU                   cpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
    SysFsDriverLegionOther::Other::GPU                   gpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
    legion::messages::CPUPower                           cpuPower;

    LOG_T(__PRETTY_FUNCTION__);


    if(!cpuPower.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(cpuPower.has_cpu_tmp_limit()) {
        setData(cpuControl.m_cpu_tmp_limit.m_current_value,cpuPower.cpu_tmp_limit().current_value());
    }
    if(cpuPower.has_cpu_clp_limit()) {
        setData(cpuControl.m_cpu_clp_limit.m_current_value,cpuPower.cpu_clp_limit().current_value());
    }
    if(cpuPower.has_cpu_ltp_limit()) {
        setData(cpuControl.m_cpu_ltp_limit.m_current_value,cpuPower.cpu_ltp_limit().current_value());
    }
    if(cpuPower.has_cpu_stp_limit()) {
        setData(cpuControl.m_cpu_stp_limit.m_current_value,cpuPower.cpu_stp_limit().current_value());
    }
    if(cpuPower.has_cpu_pl1_tau()) {
        setData(cpuControl.m_cpu_pl1_tau.m_current_value,cpuPower.cpu_pl1_tau().current_value());
    }
    if(cpuPower.has_gpu_total_onac()) {
        setData(gpuControl.m_gpu_total_onac.m_current_value,cpuPower.gpu_total_onac().current_value());
    }
    if(cpuPower.has_gpu_to_cpu_dynamic_boost()) {
        setData(gpuControl.m_gpu_to_cpu_dynamic_boost.m_current_value,cpuPower.gpu_to_cpu_dynamic_boost().current_value());
    }

    return {};
}

void SysFsDataProviderCPUPower::setValue(const std::filesystem::path &path, std::function<void (legion::messages::CPUPower::Limit::Descriptor &, uint)> setter, auto map) const
{
    if(!getData(path).trimmed().isEmpty())
    {
        auto modeValue = getData(path).split(',');

        for (unsigned int i = 0; i < modeValue.size(); ++i) {
            auto range = modeValue.at(i).split('=');

            if(range.size() == 2)
            {
                setter((*map)[range.at(0).toUInt()],range.at(1).toUInt());
            }
            else
            {
                THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid power data !").c_str());
            }
        }
    }
    else
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid power data !").c_str());
    }
}

void SysFsDataProviderCPUPower::setValuesSteps(const std::filesystem::path &path, std::function<void (legion::messages::CPUPower::Limit::Descriptor &, const QList<QString> &)> setter, auto map) const
{
    if(!getData(path).trimmed().isEmpty())
    {
        auto modeValue = getData(path).split(',');

        for (unsigned int i = 0; i < modeValue.size(); ++i) {
            auto range = modeValue.at(i).split('=');

            if(range.size() == 2)
            {
                setter((*map)[range.at(0).toUInt()],range.at(1).split("|"));
            }
            else
            {
                THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid power data !").c_str());
            }
        }
    }
    else
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid power data !").c_str());
    }
}


}
