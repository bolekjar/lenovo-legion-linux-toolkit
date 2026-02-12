// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include "SysFsDriver.h"
#include "SysFsDriverLegion.h"

namespace LenovoLegionDaemon {

class SysFsDriverLegionMachineInformation : public SysFsDriver
{
public:

    struct MachineInformation {

        explicit MachineInformation(const SysFsDriver::DescriptorType& descriptor) :
            m_bios_date(descriptor["bios_date"]),
            m_bios_release(descriptor["bios_release"]),
            m_bios_vendor(descriptor["bios_vendor"]),
            m_bios_version(descriptor["bios_version"]),
            m_board_name(descriptor["board_name"]),
            m_board_vendor(descriptor["board_vendor"]),
            m_board_version(descriptor["board_version"]),
            m_chassis_type(descriptor["chassis_type"]),
            m_chassis_vendor(descriptor["chassis_vendor"]),
            m_chassis_version(descriptor["chassis_version"]),
            m_product_family(descriptor["product_family"]),
            m_product_name(descriptor["product_name"]),
            m_product_sku(descriptor["product_sku"]),
            m_product_version(descriptor["product_version"]),
            m_sys_vendor(descriptor["sys_vendor"])
        {}

        const std::filesystem::path m_bios_date;
        const std::filesystem::path m_bios_release;
        const std::filesystem::path m_bios_vendor;
        const std::filesystem::path m_bios_version;
        const std::filesystem::path m_board_name;
        const std::filesystem::path m_board_vendor;
        const std::filesystem::path m_board_version;
        const std::filesystem::path m_chassis_type;
        const std::filesystem::path m_chassis_vendor;
        const std::filesystem::path m_chassis_version;
        const std::filesystem::path m_product_family;
        const std::filesystem::path m_product_name;
        const std::filesystem::path m_product_sku;
        const std::filesystem::path m_product_version;
        const std::filesystem::path m_sys_vendor;
    };

public:

    SysFsDriverLegionMachineInformation(QObject * parrent);

    ~SysFsDriverLegionMachineInformation() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "legion_dmi";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;


};

}
