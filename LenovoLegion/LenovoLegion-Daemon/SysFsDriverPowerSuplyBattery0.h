// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"

namespace LenovoLegionDaemon {


class SysFsDriverPowerSuplyBattery0 : public SysFsDriver
{

public:

    struct PowerSuplyBattery0 {

        PowerSuplyBattery0(const SysFsDriver::DescriptorType& descriptor) :
            m_powerSuplyBattery0(descriptor["powerSuplyBattery0"])
        {}

        const std::filesystem::path m_powerSuplyBattery0;
    };

public:

    SysFsDriverPowerSuplyBattery0(QObject* parrent);


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
};

}
