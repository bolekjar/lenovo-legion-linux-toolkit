// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderGPUPower.h"
#include "SysFsDriverLegionOther.h"

#include "../LenovoLegion-PrepareBuild/GPUPower.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderGPUPower::SysFsDataProviderGPUPower(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}


QByteArray SysFsDataProviderGPUPower::serializeAndGetData() const
{
    legion::messages::GPUPower         power;
    QByteArray                         byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFsDriverLegionOther::Other::GPU gpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));

        auto setValue = [](const std::filesystem::path& path,std::function<void (legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value)> setter,auto map)
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
                        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU power data !").c_str());
                    }
                }
            }
            else
            {
                THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU power data !").c_str());
            }
        };

        auto setValuesSteps = [](const std::filesystem::path& path,std::function<void (legion::messages::GPUPower::Limit::Descriptor &descriptor,const QList<QString>& values)> setter,auto map)
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
                        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU power data !").c_str());
                    }
                }
            }
            else
            {
                THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,std::string("Invalid CPU power data !").c_str());
            }
        };



        power.mutable_gpu_power_boost()->set_current_value(static_cast<quint8>(getData(gpuControl.m_gpu_power_boost.m_current_value).toUShort()));
        setValue(gpuControl.m_gpu_power_boost.m_default_value,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },power.mutable_gpu_power_boost()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_power_boost.m_supported,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },power.mutable_gpu_power_boost()->mutable_mode_descriptor_map());
        setValuesSteps(gpuControl.m_gpu_power_boost.m_steps,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,const QList<QString>& values){
            for (const auto& val : values) {
                descriptor.add_steps(val.toUInt());
            }
        },power.mutable_gpu_power_boost()->mutable_mode_descriptor_map());



        power.mutable_gpu_configurable_tgp()->set_current_value(static_cast<quint8>(getData(gpuControl.m_gpu_configurable_tgp.m_current_value).toUShort()));
        setValue(gpuControl.m_gpu_configurable_tgp.m_default_value,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },power.mutable_gpu_configurable_tgp()->mutable_mode_descriptor_map());
        setValue(gpuControl.m_gpu_configurable_tgp.m_supported,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },power.mutable_gpu_configurable_tgp()->mutable_mode_descriptor_map());
        setValuesSteps(gpuControl.m_gpu_configurable_tgp.m_steps,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,const QList<QString>& values){
            for (const auto& val : values) {
                descriptor.add_steps(val.toUInt());
            }
        },power.mutable_gpu_configurable_tgp()->mutable_mode_descriptor_map());




        power.mutable_gpu_temperature_limit()->set_current_value(static_cast<quint8>(getData(gpuControl.m_gpu_temperature_limit.m_current_value).toUShort()));
        setValue(gpuControl.m_gpu_temperature_limit.m_default_value,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_default_value(value);
        },power.mutable_gpu_temperature_limit()->mutable_mode_descriptor_map());

        setValue(gpuControl.m_gpu_temperature_limit.m_max_value,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_max_value(value);
        },power.mutable_gpu_temperature_limit()->mutable_mode_descriptor_map());

        setValue(gpuControl.m_gpu_temperature_limit.m_min_value,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_min_value(value);
        },power.mutable_gpu_temperature_limit()->mutable_mode_descriptor_map());

        setValue(gpuControl.m_gpu_temperature_limit.m_scalar_increment,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_scalar_increment(value);
        },power.mutable_gpu_temperature_limit()->mutable_mode_descriptor_map());

        setValue(gpuControl.m_gpu_temperature_limit.m_supported,[&](legion::messages::GPUPower::Limit::Descriptor &descriptor,uint value){
            descriptor.set_supported(value);
        },power.mutable_gpu_temperature_limit()->mutable_mode_descriptor_map());




    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            power.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(power.ByteSizeLong());
    if(!power.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderGPUPower::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverLegionOther::Other::GPU gpuControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionOther::DRIVER_NAME));
    legion::messages::GPUPower         power;

    LOG_T(__PRETTY_FUNCTION__);


    if(!power.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(power.has_gpu_power_boost()) {
        setData(gpuControl.m_gpu_power_boost.m_current_value,power.gpu_power_boost().current_value());
    }
    if(power.has_gpu_configurable_tgp()) {
        setData(gpuControl.m_gpu_configurable_tgp.m_current_value,power.gpu_configurable_tgp().current_value());
    }
    if(power.has_gpu_temperature_limit()) {
        setData(gpuControl.m_gpu_temperature_limit.m_current_value,power.gpu_temperature_limit().current_value());
    }

    return {};
}


}
