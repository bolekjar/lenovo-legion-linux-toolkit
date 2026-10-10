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

class SysFsDriverCPU : public SysFsDriver
{

public:

    struct CPU {

        struct Topology {

            Topology(const SysFsDriver::DescriptorType& descriptor) :
                m_kernelMax(descriptor["kernelMax"]),
                m_possible(descriptor["possible"]),
                m_offline(descriptor["offline"]),
                m_online(descriptor["online"]),
                m_present(descriptor["present"])
            {}


            /*
             * CPU topology
             */
            const std::filesystem::path m_kernelMax;  //The maximum CPU index allowed by the kernel configuration. [NR_CPUS-1]
            const std::filesystem::path m_possible;   //CPUs that have been allocated resources and can be brought online if they are present. [cpu_possible_mask]
            const std::filesystem::path m_offline;    //CPUs that are not online because they have been HOTPLUGGED off (see cpu-hotplug.txt) or exceed the limit of CPUs allowed by the kernel configuration (kernel_max above). [~cpu_online_mask + cpus >= NR_CPUS]
            const std::filesystem::path m_online;     //CPUs that are online and being scheduled
            const std::filesystem::path m_present;    //CPUs that have been identified as being present in the system. [cpu_present_mask]
        };

        struct Smt {

            Smt(const SysFsDriver::DescriptorType& descriptor) :
                m_active(descriptor.find("smtActive") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["smtActive"]),
                m_control(descriptor.find("smtControl") == descriptor.end() ? std::optional<std::filesystem::path>() : descriptor["smtControl"])
            {
                if(!m_active.has_value() || !m_active.has_value())
                {
                    THROW_EXCEPTION(exception_T,ERROR_CODES::DRIVER_NOT_AVAILABLE,"Driver " + std::string(DRIVER_NAME) + " is not loaded !");
                }
            }

            const std::optional<std::filesystem::path> m_active;
            const std::optional<std::filesystem::path> m_control;
        };


        explicit CPU(const SysFsDriver::DescriptorType& descriptor) :
            m_topology(descriptor),
            m_smt(descriptor)
        {}

        const Topology       m_topology;
        const Smt            m_smt;
    };

public:

    SysFsDriverCPU(QObject * parrent);


    ~SysFsDriverCPU() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME = "__system_cpu__";

private:

};

}
