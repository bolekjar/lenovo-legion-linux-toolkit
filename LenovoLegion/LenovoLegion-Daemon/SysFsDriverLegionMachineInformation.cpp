// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDriverLegionMachineInformation.h"

#include <Core/LoggerHolder.h>

#include <QFile>
#include <QTextStream>



namespace LenovoLegionDaemon {




SysFsDriverLegionMachineInformation::SysFsDriverLegionMachineInformation(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-dmi/machine-information-0/",{},parrent,MODULE_NAME)
{

}

void SysFsDriverLegionMachineInformation::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(m_path))
    {
        LOG_D(QString("Found Legion DMI Machine Information driver in path: ") + m_path.c_str());

        m_descriptor["bios_date"] = std::filesystem::path(m_path).append("bios_date");
        m_descriptor["bios_release"] = std::filesystem::path(m_path).append("bios_release");
        m_descriptor["bios_vendor"] = std::filesystem::path(m_path).append("bios_vendor");
        m_descriptor["bios_version"] = std::filesystem::path(m_path).append("bios_version");
        m_descriptor["board_name"] = std::filesystem::path(m_path).append("board_name");
        m_descriptor["board_vendor"] = std::filesystem::path(m_path).append("board_vendor");
        m_descriptor["board_version"] = std::filesystem::path(m_path).append("board_version");
        m_descriptor["chassis_type"] = std::filesystem::path(m_path).append("chassis_type");
        m_descriptor["chassis_vendor"] = std::filesystem::path(m_path).append("chassis_vendor");
        m_descriptor["chassis_version"] = std::filesystem::path(m_path).append("chassis_version");
        m_descriptor["product_family"] = std::filesystem::path(m_path).append("product_family");
        m_descriptor["product_name"] = std::filesystem::path(m_path).append("product_name");
        m_descriptor["product_sku"] = std::filesystem::path(m_path).append("product_sku");
        m_descriptor["product_version"] = std::filesystem::path(m_path).append("product_version");
        m_descriptor["sys_vendor"] = std::filesystem::path(m_path).append("sys_vendor");
    }
}

}
