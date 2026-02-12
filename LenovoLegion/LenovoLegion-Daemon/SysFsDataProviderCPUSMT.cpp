// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderCPUSMT.h"
#include "SysFsDriverCPU.h"
#include "SysFsDriverCPUXList.h"

#include "../LenovoLegion-PrepareBuild/CPUOptions.pb.h"


#include <Core/LoggerHolder.h>
#include <QCoreApplication>

namespace LenovoLegionDaemon {

SysFsDataProviderCPUSMT::SysFsDataProviderCPUSMT(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderCPUSMT::serializeAndGetData() const
{
    legion::messages::CPUSMT       cpuSmt;
    QByteArray                     byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverCPU::CPU::Smt smtControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));

        cpuSmt.set_control(getData(smtControl.m_control.value()).toStdString());
        cpuSmt.set_active(getData(smtControl.m_active.value()).toUShort() == 1);
    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            cpuSmt.Clear();
        }
        else
        {
            throw;
        }
    }

    byteArray.resize(cpuSmt.ByteSizeLong());
    if(!cpuSmt.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderCPUSMT::deserializeAndSetData(const QByteArray &data)
{
    SysFsDriverCPU::CPU::Smt smtControl(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverCPU::DRIVER_NAME));
    legion::messages::CPUSMT cpuSmt;

    m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,true);
    auto cleanup =  qScopeGuard([this] { m_sysFsDriverManager->blockKernelEvent(SysFsDriverCPUXList::DRIVER_NAME,false); });
    LOG_T(__PRETTY_FUNCTION__);

    if(!cpuSmt.ParseFromArray(data.data(),data.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::INVALID_DATA,"Parse of data message error !");
    }

    if(cpuSmt.has_control()) {
        setData(smtControl.m_control.value(),cpuSmt.control());
    }

    m_sysFsDriverManager->processAllUdevEvents(100);

    m_sysFsDriverManager->refreshDriver(SysFsDriverCPUXList::DRIVER_NAME);
    return {};
}


}
