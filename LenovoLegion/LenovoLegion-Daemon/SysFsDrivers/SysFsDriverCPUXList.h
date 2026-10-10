// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"


#include <optional>


namespace LenovoLegionDaemon {

class SysFsDriverCPUXList : public SysFsDriver
{
public:

    struct CPUXList {

        struct CPUX {

            struct CPUXFreq {

                CPUXFreq(const SysFsDriver::DescriptorType& descriptor) :
                    m_affectedCpus(descriptor["affectedCpus"]),
                    m_cpuBaseFreq((descriptor.find("cpuBaseFreq") == descriptor.end()) ? std::optional<std::filesystem::path>() : descriptor["cpuBaseFreq"]),
                    m_cpuInfoMinFreq(descriptor["cpuInfoMinFreq"]),
                    m_cpuInfoMaxFreq(descriptor["cpuInfoMaxFreq"]),
                    m_cpuScalingAvailableGovernors(descriptor["cpuScalingAvailableGovernors"]),
                    m_cpuScalingGovernor(descriptor["cpuScalingGovernor"]),
                    m_cpuScalingCurFreq(descriptor["cpuScalingCurFreq"]),
                    m_cpuScalingMinFreq(descriptor["cpuScalingMinFreq"]),
                    m_cpuScalingMaxFreq(descriptor["cpuScalingMaxFreq"])
                {}

                const std::filesystem::path                m_affectedCpus;                    //List of online CPUs belonging to this policy (i.e. sharing the hardware performance scaling interface represented by the policyX policy object).
                const std::optional<std::filesystem::path> m_cpuBaseFreq;                     //Base operating frequency the CPUs belonging to this policy can run at (in kHz).
                const std::filesystem::path                m_cpuInfoMinFreq;                  //Minimum possible operating frequency the CPUs belonging to this policy can run at (in kHz).
                const std::filesystem::path                m_cpuInfoMaxFreq;                  //Maximum possible operating frequency the CPUs belonging to this policy can run at (in kHz).
                const std::filesystem::path                m_cpuScalingAvailableGovernors;    //List of CPUFreq scaling governors present in the kernel that can be attached to this policy or (if the intel_pstate scaling driver is in use) list of scaling algorithms provided by the driver that can be applied to this policy.
                const std::filesystem::path                m_cpuScalingGovernor;              //The scaling governor currently attached to this policy or (if the intel_pstate scaling driver is in use) the scaling algorithm provided by the driver that is currently applied to this policy.
                const std::filesystem::path                m_cpuScalingCurFreq;               //Current frequency of all of the CPUs belonging to this policy (in kHz).
                const std::filesystem::path                m_cpuScalingMinFreq;               //Minimum frequency the CPUs belonging to this policy are allowed to be running at (in kHz).
                const std::filesystem::path                m_cpuScalingMaxFreq;               //Maximum frequency the CPUs belonging to this policy are allowed to be running at (in kHz).
            };

            struct CPUXTopology {

                CPUXTopology(const SysFsDriver::DescriptorType& descriptor) :
                    m_clusterId(descriptor["clusterId"]),
                    m_physicalPackageId(descriptor["physicalPackageId"]),
                    m_coreId(descriptor["coreId"]),
                    m_dieId(descriptor["dieId"]),
                    m_clusterCpusList(descriptor["clusterCpusList"]),
                    m_packageCpusList(descriptor["packageCpusList"]),
                    m_dieCpusList(descriptor["dieCpusList"]),
                    m_coreCpusList(descriptor["coreCpusList"]),
                    m_coreSiblingsList(descriptor["coreSiblingsList"]),
                    m_threadSiblingsList(descriptor["threadSiblingsList"])
                {}

                const std::filesystem::path m_clusterId;          //The CPU cluster ID of cpuX. Typically it is the hardware platform’s identifier (rather than the kernel’s). The actual value is architecture and platform dependent.
                const std::filesystem::path m_physicalPackageId;  //Physical package id of cpuX. Typically corresponds to a physical socket number, but the actual value is architecture and platform dependent.
                const std::filesystem::path m_coreId;             //The CPU core ID of cpuX. Typically it is the hardware platform’s identifier (rather than the kernel’s). The actual value is architecture and platform dependent.
                const std::filesystem::path m_dieId;              //The CPU die ID of cpuX. Typically it is the hardware platform’s identifier (rather than the kernel’s). The actual value is architecture and platform dependent.

                const std::filesystem::path m_clusterCpusList;    //Human-readable list of CPUs sharing the same cluster_id.
                const std::filesystem::path m_packageCpusList;    //Human-readable list of CPUs sharing the same physical_package_id. (deprecated name: “core_siblings_list”)
                const std::filesystem::path m_dieCpusList;        //Human-readable list of CPUs within the same die.
                const std::filesystem::path m_coreCpusList;       //Human-readable list of CPUs within the same core. (deprecated name: “thread_siblings_list”);
                const std::filesystem::path m_coreSiblingsList;   //Human-readable list of cpuX's hardware threads within the same
                const std::filesystem::path m_threadSiblingsList; //Human-readable list of cpuX's hardware threads within the same core as cpuX.
            };

            CPUX(const SysFsDriver::DescriptorType& descriptor) :
                m_freq(descriptor),
                m_topology(descriptor.find("clusterId") == descriptor.end()         ||
                           descriptor.find("physicalPackageId") == descriptor.end() ||
                           descriptor.find("coreId") == descriptor.end()            ||
                           descriptor.find("dieId") == descriptor.end()             ||
                           descriptor.find("clusterCpusList") == descriptor.end()   ||
                           descriptor.find("packageCpusList") == descriptor.end()   ||
                           descriptor.find("dieCpusList") == descriptor.end()       ||
                           descriptor.find("coreCpusList") == descriptor.end()      ||
                           descriptor.find("coreSiblingsList") == descriptor.end()  ||
                           descriptor.find("threadSiblingsList") == descriptor.end()
                           ? std::nullopt : std::make_optional(CPUXTopology(descriptor))),
                m_cpuOnline((descriptor.find("cpuOnline") == descriptor.end()) ? std::optional<std::filesystem::path>() : descriptor["cpuOnline"])
            {}

            CPUX(const CPUX& ) = default;

            CPUXFreq                           m_freq;
            const std::optional<CPUXTopology>  m_topology;


            bool isOnlineAvailable() const
            {
                return  m_cpuOnline.has_value();
            }

            const std::optional<std::filesystem::path> m_cpuOnline;
        };


        CPUXList(const SysFsDriver::DescriptorsInVectorType& descriptorsInVector)
        {
            for (const auto& descriptor : descriptorsInVector) {
                m_cpus.emplace_back(descriptor);
            }
        }

        const std::vector<CPUX>& cpuList() const
        {
            return m_cpus;
        }

    private:

        std::vector<CPUX> m_cpus; //List of CPUs
    };;

public:

    SysFsDriverCPUXList(QObject* parrent);


    ~SysFsDriverCPUXList() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;


    virtual void handleKernelEvent(const KernelEvent::Event& event) override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "processor";
};

}
