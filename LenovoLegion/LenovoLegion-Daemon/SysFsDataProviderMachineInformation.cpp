// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderMachineInformation.h"
#include "SysFsDriverLegionMachineInformation.h"


#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"


#include <Core/LoggerHolder.h>


namespace LenovoLegionDaemon {

SysFsDataProviderMachineInformation::SysFsDataProviderMachineInformation(SysFsDriverManager* sysFsDriverManager,QObject* parent) : SysFsDataProvider(sysFsDriverManager,parent,dataType) {}



QByteArray SysFsDataProviderMachineInformation::serializeAndGetData() const
{
    legion::messages::MachineInformation   machineInfo;
    QByteArray                             byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverLegionMachineInformation::MachineInformation info(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionMachineInformation::DRIVER_NAME));

        machineInfo.set_bios_date(getData(info.m_bios_date).toStdString());
        machineInfo.set_bios_release(getData(info.m_bios_release).toStdString());
        machineInfo.set_bios_vendor(getData(info.m_bios_vendor).toStdString());
        machineInfo.set_bios_version(getData(info.m_bios_version).toStdString());
        machineInfo.set_board_name(getData(info.m_board_name).toStdString());
        machineInfo.set_board_vendor(getData(info.m_board_vendor).toStdString());
        machineInfo.set_board_version(getData(info.m_board_version).toStdString());
        machineInfo.set_chassis_type(getData(info.m_chassis_type).toStdString());
        machineInfo.set_chassis_vendor(getData(info.m_chassis_vendor).toStdString());
        machineInfo.set_chassis_version(getData(info.m_chassis_version).toStdString());
        machineInfo.set_product_family(getData(info.m_product_family).toStdString());
        machineInfo.set_product_name(getData(info.m_product_name).toStdString());
        machineInfo.set_product_sku(getData(info.m_product_sku).toStdString());
        machineInfo.set_product_version(getData(info.m_product_version).toStdString());
        machineInfo.set_sys_vendor(getData(info.m_sys_vendor).toStdString());

    } catch(SysFsDriver::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
            machineInfo.Clear();
        }
        else
        {
            throw;
        }
    }


    byteArray.resize(machineInfo.ByteSizeLong());
    if(!machineInfo.SerializeToArray(byteArray.data(),byteArray.size()))
    {
        THROW_EXCEPTION(exception_T,DataProvider::ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return byteArray;
}

QByteArray SysFsDataProviderMachineInformation::deserializeAndSetData(const QByteArray &)
{
    return {};
}


}
