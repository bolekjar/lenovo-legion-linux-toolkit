// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../SysFsDriver.h"

namespace LenovoLegionDaemon::Kit {


class SysFsDriverPowerSuplyBattery0 : public SysFsDriver
{

public:

    /*
     * Files of BAT0 (status always, the others only when the battery reports them: empty path otherwise) and online
     * of the AC adapter (power_supply of type Mains, empty path without one)
     */
    struct PowerSuplyBattery0 {

        PowerSuplyBattery0(const SysFsDriver::DescriptorType& descriptor) :
            m_powerSuplyBattery0(descriptor["powerSuplyBattery0"]),
            m_capacity(descriptor.value("capacity")),
            m_capacityLevel(descriptor.value("capacity_level")),
            m_energyNow(descriptor.value("energy_now")),
            m_energyFull(descriptor.value("energy_full")),
            m_energyFullDesign(descriptor.value("energy_full_design")),
            m_chargeNow(descriptor.value("charge_now")),
            m_chargeFull(descriptor.value("charge_full")),
            m_chargeFullDesign(descriptor.value("charge_full_design")),
            m_powerNow(descriptor.value("power_now")),
            m_currentNow(descriptor.value("current_now")),
            m_voltageNow(descriptor.value("voltage_now")),
            m_voltageMinDesign(descriptor.value("voltage_min_design")),
            m_cycleCount(descriptor.value("cycle_count")),
            m_technology(descriptor.value("technology")),
            m_manufacturer(descriptor.value("manufacturer")),
            m_modelName(descriptor.value("model_name")),
            m_acOnline(descriptor.value("ac_online"))
        {}

        const std::filesystem::path m_powerSuplyBattery0;
        const std::filesystem::path m_capacity;
        const std::filesystem::path m_capacityLevel;
        const std::filesystem::path m_energyNow;
        const std::filesystem::path m_energyFull;
        const std::filesystem::path m_energyFullDesign;
        const std::filesystem::path m_chargeNow;
        const std::filesystem::path m_chargeFull;
        const std::filesystem::path m_chargeFullDesign;
        const std::filesystem::path m_powerNow;
        const std::filesystem::path m_currentNow;
        const std::filesystem::path m_voltageNow;
        const std::filesystem::path m_voltageMinDesign;
        const std::filesystem::path m_cycleCount;
        const std::filesystem::path m_technology;
        const std::filesystem::path m_manufacturer;
        const std::filesystem::path m_modelName;
        const std::filesystem::path m_acOnline;
    };

public:

    SysFsDriverPowerSuplyBattery0();


    ~SysFsDriverPowerSuplyBattery0() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;


    /*
     *
     */
    virtual void handleKernelEvent(const KernelEvent::Event&) override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "power_supply";

private:

    /*
     * Name of the AC adapter (power_supply of type Mains), empty without one
     */
    QString m_adapterName;
};

}
