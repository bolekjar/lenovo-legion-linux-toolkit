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


class SysFsDriverIntelPowercapRapl : public SysFsDriver
{

public:

    struct IntelPowercapRapl {

        IntelPowercapRapl(const SysFsDriver::DescriptorType& descriptor) :
            m_ltp_max_power_uw(descriptor["ltp_max_power_uw"]),
            m_ltp_time_window_us(descriptor["ltp_time_window_us"]),
            m_ltp_name(descriptor["ltp_name"]),
            m_ltp_power_limit_uw(descriptor["ltp_power_limit_uw"]),
            m_stp_max_power_uw(descriptor["stp_max_power_uw"]),
            m_stp_time_window_us(descriptor["stp_time_window_us"]),
            m_stp_name(descriptor["stp_name"]),
            m_stp_power_limit_uw(descriptor["stp_power_limit_uw"]),
            m_pp_max_power_uw(descriptor["pp_max_power_uw"]),
            m_pp_time_window_us(descriptor["pp_time_window_us"]),
            m_pp_name(descriptor["pp_name"]),
            m_pp_power_limit_uw(descriptor["pp_power_limit_uw"]),
            m_powercapCPUEnergy(descriptor["powercapCPUEnergy"]),
            m_max_energy_range(descriptor["max_energy_range"]),
            m_enabled(descriptor["enabled"])
        {}

        const std::filesystem::path m_ltp_max_power_uw;
        const std::filesystem::path m_ltp_time_window_us;
        const std::filesystem::path m_ltp_name;
        const std::filesystem::path m_ltp_power_limit_uw;

        const std::filesystem::path m_stp_max_power_uw;
        const std::filesystem::path m_stp_time_window_us;
        const std::filesystem::path m_stp_name;
        const std::filesystem::path m_stp_power_limit_uw;

        const std::filesystem::path m_pp_max_power_uw;
        const std::filesystem::path m_pp_time_window_us;
        const std::filesystem::path m_pp_name;
        const std::filesystem::path m_pp_power_limit_uw;

        const std::filesystem::path m_powercapCPUEnergy;
        const std::filesystem::path m_max_energy_range;
        const std::filesystem::path m_enabled;
    };


    struct IntelPowercapRaplMMIO {

        IntelPowercapRaplMMIO(const SysFsDriver::DescriptorType& descriptor) :
            m_ltp_max_power_uw(descriptor["mmio_ltp_max_power_uw"]),
            m_ltp_time_window_us(descriptor["mmio_ltp_time_window_us"]),
            m_ltp_name(descriptor["mmio_ltp_name"]),
            m_ltp_power_limit_uw(descriptor["mmio_ltp_power_limit_uw"]),
            m_stp_max_power_uw(descriptor["mmio_stp_max_power_uw"]),
            m_stp_time_window_us(descriptor["mmio_stp_time_window_us"]),
            m_stp_name(descriptor["mmio_stp_name"]),
            m_stp_power_limit_uw(descriptor["mmio_stp_power_limit_uw"]),
            m_pp_max_power_uw(descriptor["mmio_pp_max_power_uw"]),
            m_pp_time_window_us(descriptor["mmio_pp_time_window_us"]),
            m_pp_name(descriptor["mmio_pp_name"]),
            m_pp_power_limit_uw(descriptor["mmio_pp_power_limit_uw"]),
            m_powercapCPUEnergy(descriptor["mmio_powercapCPUEnergy"]),
            m_max_energy_range(descriptor["mmio_max_energy_range"]),
            m_enabled(descriptor["mmio_enabled"])
        {}

        const std::filesystem::path m_ltp_max_power_uw;
        const std::filesystem::path m_ltp_time_window_us;
        const std::filesystem::path m_ltp_name;
        const std::filesystem::path m_ltp_power_limit_uw;

        const std::filesystem::path m_stp_max_power_uw;
        const std::filesystem::path m_stp_time_window_us;
        const std::filesystem::path m_stp_name;
        const std::filesystem::path m_stp_power_limit_uw;

        const std::filesystem::path m_pp_max_power_uw;
        const std::filesystem::path m_pp_time_window_us;
        const std::filesystem::path m_pp_name;
        const std::filesystem::path m_pp_power_limit_uw;

        const std::filesystem::path m_powercapCPUEnergy;
        const std::filesystem::path m_max_energy_range;
        const std::filesystem::path m_enabled;
    };
public:

    SysFsDriverIntelPowercapRapl(QObject* parrent);


    ~SysFsDriverIntelPowercapRapl() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;

    /*
     * Handle kernel event
     */
    virtual void handleKernelEvent(const KernelEvent::Event &event) override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "intel-rapl";
};

}
