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

class SysFsDriverCPUInfo : public SysFsDriver
{
public:

    struct CPUInfo {

        struct Cache {

            explicit Cache(const SysFsDriver::DescriptorType& descriptor) :
                m_level(descriptor["cache_level"]),
                m_type(descriptor["cache_type"]),
                m_size(descriptor["cache_size"])
            {}

            const std::filesystem::path m_level;
            const std::filesystem::path m_type;
            const std::filesystem::path m_size;
        };

        explicit CPUInfo(const SysFsDriver::DescriptorType& descriptor,const SysFsDriver::DescriptorsInVectorType& caches) :
            m_modalias(descriptor["modalias"]),
            m_package_id(descriptor["package_id"]),
            m_caches([&caches](){
                std::vector<Cache> caches_vector;
                for (const auto& cache_descriptor : caches) {
                    caches_vector.emplace_back(cache_descriptor);
                }
                return caches_vector;
            }())
        {}

        const std::filesystem::path m_modalias;
        const std::filesystem::path m_package_id;
        const std::vector<Cache>    m_caches;
    };

public:

    explicit SysFsDriverCPUInfo(QObject *parent );
    ~SysFsDriverCPUInfo() override = default;


    void init() override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "__system_cpu_info__";
};

}
