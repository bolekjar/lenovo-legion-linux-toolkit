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


class SysFsDriverCPUAtom : public SysFsDriver
{

public:

    struct CPUAtom {

        CPUAtom(const SysFsDriver::DescriptorType& descriptor) :
            m_cpus(descriptor["cpus"])
        {}

        const std::filesystem::path m_cpus;
    };

public:

    SysFsDriverCPUAtom(QObject* parrent);


    ~SysFsDriverCPUAtom() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "__cpu_atom__";
};

}
