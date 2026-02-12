// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderIntelMSR.h"
#include "SysFSDriverLegionIntelMSR.h"


#include "../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.h"


#include "Core/LoggerHolder.h"


namespace LenovoLegionDaemon {

SysFsDataProviderIntelMSR::SysFsDataProviderIntelMSR(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}

QByteArray SysFsDataProviderIntelMSR::serializeAndGetData() const
{
    legion::messages::CpuIntelMSR cpuIntelMSRMessage;;
    QByteArray                    byteArray;

    LOG_T(__PRETTY_FUNCTION__);


    try {
        SysFSDriverLegionIntelMSR::IntelMSR intelMSR(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionIntelMSR::DRIVER_NAME));

        cpuIntelMSRMessage.mutable_analogio()->set_max_overvolt(getData(intelMSR.m_analogio_max_overvolt).toInt());
        cpuIntelMSRMessage.mutable_analogio()->set_max_undervolt(getData(intelMSR.m_analogio_max_undervolt).toInt());
        cpuIntelMSRMessage.mutable_analogio()->set_offset(getData(intelMSR.m_analogio_offset).toInt());
        cpuIntelMSRMessage.mutable_analogio()->set_supported(getData(intelMSR.m_analogio_offset_ctrl_supported).toUShort() == 1);


        cpuIntelMSRMessage.mutable_cache()->set_max_overvolt(getData(intelMSR.m_cache_max_overvolt).toInt());
        cpuIntelMSRMessage.mutable_cache()->set_max_undervolt(getData(intelMSR.m_cache_max_undervolt).toInt());
        cpuIntelMSRMessage.mutable_cache()->set_offset(getData(intelMSR.m_cache_offset).toInt());
        cpuIntelMSRMessage.mutable_cache()->set_supported(getData(intelMSR.m_cache_offset_ctrl_supported).toUShort() == 1);



        cpuIntelMSRMessage.mutable_cpu()->set_max_overvolt(getData(intelMSR.m_cpu_max_overvolt).toInt());
        cpuIntelMSRMessage.mutable_cpu()->set_max_undervolt(getData(intelMSR.m_cpu_max_undervolt).toInt());
        cpuIntelMSRMessage.mutable_cpu()->set_offset(getData(intelMSR.m_cpu_offset).toInt());
        cpuIntelMSRMessage.mutable_cpu()->set_supported(getData(intelMSR.m_cpu_offset_ctrl_supported).toUShort() == 1);

        cpuIntelMSRMessage.mutable_gpu()->set_max_overvolt(getData(intelMSR.m_gpu_max_overvolt).toInt());
        cpuIntelMSRMessage.mutable_gpu()->set_max_undervolt(getData(intelMSR.m_gpu_max_undervolt).toInt());
        cpuIntelMSRMessage.mutable_gpu()->set_offset(getData(intelMSR.m_gpu_offset).toInt());
        cpuIntelMSRMessage.mutable_gpu()->set_supported(getData(intelMSR.m_gpu_offset_ctrl_supported).toUShort() == 1);

        cpuIntelMSRMessage.mutable_uncore()->set_max_overvolt(getData(intelMSR.m_uncore_max_overvolt).toInt());
        cpuIntelMSRMessage.mutable_uncore()->set_max_undervolt(getData(intelMSR.m_uncore_max_undervolt).toInt());
        cpuIntelMSRMessage.mutable_uncore()->set_offset(getData(intelMSR.m_uncore_offset).toInt());
        cpuIntelMSRMessage.mutable_uncore()->set_supported(getData(intelMSR.m_uncore_offset_ctrl_supported).toUShort() == 1);
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- IntelMSR Driver not available");
            cpuIntelMSRMessage.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(cpuIntelMSRMessage.ByteSizeLong());
    if(!cpuIntelMSRMessage.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderIntelMSR::deserializeAndSetData(const QByteArray &data)
{
    SysFSDriverLegionIntelMSR::IntelMSR intelMSR(m_sysFsDriverManager->getDriverDesriptor(SysFSDriverLegionIntelMSR::DRIVER_NAME));
    legion::messages::CpuIntelMSR cpuIntelMSRMessage;

    LOG_T(QString(__PRETTY_FUNCTION__));

    // Skip if buffer is empty
    if(data.isEmpty() || data.size() == 0) {
        LOG_D("SysFsDataProviderIntelMSR::deserializeAndSetData - empty data, skipping");
        return {};
    }

    if(!cpuIntelMSRMessage.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    // Apply only fields that are present
    if(cpuIntelMSRMessage.has_analogio() && cpuIntelMSRMessage.analogio().has_offset()) {
        setData(intelMSR.m_analogio_offset, cpuIntelMSRMessage.analogio().offset());
    }
    if(cpuIntelMSRMessage.has_cache() && cpuIntelMSRMessage.cache().has_offset()) {
        setData(intelMSR.m_cache_offset, cpuIntelMSRMessage.cache().offset());
    }
    if(cpuIntelMSRMessage.has_cpu() && cpuIntelMSRMessage.cpu().has_offset()) {
        setData(intelMSR.m_cpu_offset, cpuIntelMSRMessage.cpu().offset());
    }
    if(cpuIntelMSRMessage.has_gpu() && cpuIntelMSRMessage.gpu().has_offset()) {
        setData(intelMSR.m_gpu_offset, cpuIntelMSRMessage.gpu().offset());
    }
    if(cpuIntelMSRMessage.has_uncore() && cpuIntelMSRMessage.uncore().has_offset()) {
        setData(intelMSR.m_uncore_offset, cpuIntelMSRMessage.uncore().offset());
    }

    return {};
}

}

