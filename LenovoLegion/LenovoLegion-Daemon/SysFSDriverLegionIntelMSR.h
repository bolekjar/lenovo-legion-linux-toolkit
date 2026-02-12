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

class SysFSDriverLegionIntelMSR : public SysFsDriver
{
public:

    struct IntelMSR {

        IntelMSR(const SysFsDriver::DescriptorType& descriptor) :
            m_analogio_max_overvolt(descriptor["analogio_max_overvolt"]),
            m_analogio_max_undervolt(descriptor["analogio_max_undervolt"]),
            m_analogio_offset(descriptor["analogio_offset"]),
            m_analogio_offset_ctrl_supported(descriptor["analogio_offset_ctrl_supported"]),

            m_cache_max_overvolt(descriptor["cache_max_overvolt"]),
            m_cache_max_undervolt(descriptor["cache_max_undervolt"]),
            m_cache_offset(descriptor["cache_offset"]),
            m_cache_offset_ctrl_supported(descriptor["cache_offset_ctrl_supported"]),

            m_cpu_max_overvolt(descriptor["cpu_max_overvolt"]),
            m_cpu_max_undervolt(descriptor["cpu_max_undervolt"]),
            m_cpu_offset(descriptor["cpu_offset"]),
            m_cpu_offset_ctrl_supported(descriptor["cpu_offset_ctrl_supported"]),

            m_gpu_max_overvolt(descriptor["gpu_max_overvolt"]),
            m_gpu_max_undervolt(descriptor["gpu_max_undervolt"]),
            m_gpu_offset(descriptor["gpu_offset"]),
            m_gpu_offset_ctrl_supported(descriptor["gpu_offset_ctrl_supported"]),

            m_uncore_max_overvolt(descriptor["uncore_max_overvolt"]),
            m_uncore_max_undervolt(descriptor["uncore_max_undervolt"]),
            m_uncore_offset(descriptor["uncore_offset"]),
            m_uncore_offset_ctrl_supported(descriptor["uncore_offset_ctrl_supported"])
        {}

        const std::filesystem::path      m_analogio_max_overvolt;
        const std::filesystem::path      m_analogio_max_undervolt;
        const std::filesystem::path      m_analogio_offset;
        const std::filesystem::path      m_analogio_offset_ctrl_supported;

        const std::filesystem::path      m_cache_max_overvolt;
        const std::filesystem::path      m_cache_max_undervolt;
        const std::filesystem::path      m_cache_offset;
        const std::filesystem::path      m_cache_offset_ctrl_supported;

        const std::filesystem::path      m_cpu_max_overvolt;
        const std::filesystem::path      m_cpu_max_undervolt;
        const std::filesystem::path      m_cpu_offset;
        const std::filesystem::path      m_cpu_offset_ctrl_supported;

        const std::filesystem::path      m_gpu_max_overvolt;
        const std::filesystem::path      m_gpu_max_undervolt;
        const std::filesystem::path      m_gpu_offset;
        const std::filesystem::path      m_gpu_offset_ctrl_supported;

        const std::filesystem::path      m_uncore_max_overvolt;
        const std::filesystem::path      m_uncore_max_undervolt;
        const std::filesystem::path      m_uncore_offset;
        const std::filesystem::path      m_uncore_offset_ctrl_supported;
    };


public:

    SysFSDriverLegionIntelMSR(QObject * parrent);

    ~SysFSDriverLegionIntelMSR() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "legion_intel_msr";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;
};

}
