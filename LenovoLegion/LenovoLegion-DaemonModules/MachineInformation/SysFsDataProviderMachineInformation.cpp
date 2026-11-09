// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDataProviderMachineInformation.h"
#include <Drivers/SysFsDriverLegionMachineInformation.h>


#include <ComputerInfo.pb.h>


#include <KitLog.h>


namespace LenovoLegionDaemon::Kit {

SysFsDataProviderMachineInformation::SysFsDataProviderMachineInformation(SysFsDriverManager* sysFsDriverManager) : SysFsDataProvider(sysFsDriverManager,dataType) {}



QByteArray SysFsDataProviderMachineInformation::serializeAndGetData()
{
    legion::messages::MachineInformation   machineInfo;
    QByteArray                             byteArray;

    LOG_T(__PRETTY_FUNCTION__);

    try {
        SysFsDriverLegionMachineInformation::MachineInformation info(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionMachineInformation::DRIVER_NAME));

        /*
         * A field is missing when the BIOS does not fill it or the kernel is older (/sys/class/dmi/id: 2.6.23,
         * product_family 4.12, product_sku 4.18, bios_release and ec_firmware_release 5.8): empty value
         */
        const auto text = [](const std::filesystem::path& path) {
            return getOptionalData(path).value_or(QString()).toStdString();
        };

        machineInfo.set_bios_date(text(info.m_bios_date));
        machineInfo.set_bios_release(text(info.m_bios_release));
        machineInfo.set_bios_vendor(text(info.m_bios_vendor));
        machineInfo.set_bios_version(text(info.m_bios_version));
        machineInfo.set_board_name(text(info.m_board_name));
        machineInfo.set_board_vendor(text(info.m_board_vendor));
        machineInfo.set_board_version(text(info.m_board_version));
        machineInfo.set_chassis_type(text(info.m_chassis_type));
        machineInfo.set_chassis_vendor(text(info.m_chassis_vendor));
        machineInfo.set_chassis_version(text(info.m_chassis_version));
        machineInfo.set_product_family(text(info.m_product_family));
        machineInfo.set_product_name(text(info.m_product_name));
        machineInfo.set_product_sku(text(info.m_product_sku));
        machineInfo.set_product_version(text(info.m_product_version));
        machineInfo.set_sys_vendor(text(info.m_sys_vendor));

    } catch(SysFsDriver::exception_T& ex)
    {
        /*
         * DMI not available (no /sys/class/dmi/id): an error, not an empty message. The GUI shows the data type as
         * not available (pages grayed, Help system check).
         */
        if(ex.errcodeInfo().value() == SysFsDriver::ERROR_CODES::DRIVER_NOT_AVAILABLE)
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + "- Driver not available");
        }

        throw;
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
