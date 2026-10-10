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


class SysFsDriverLegionOther : public SysFsDriver
{
public:

    enum LegionVmiOtherEventType : int {
        LENOVO_WMI_OTHER_CPU_STP			= 1000,
        LENOVO_WMI_OTHER_CPU_LTP			= 1001,
        LENOVO_WMI_OTHER_CPU_PP				= 1002,
        LENOVO_WMI_OTHER_CPU_TMP			= 1003,
        LENOVO_WMI_OTHER_APU_PPT			= 1004,
        LENOVO_WMI_OTHER_CPU_CLP			= 1005,
        LENOVO_WMI_OTHER_CPU_PL1_TAU		= 1006,
        LENOVO_WMI_OTHER_GPU_PB				= 1007,
        LENOVO_WMI_OTHER_GPU_TGP			= 1008,
        LENOVO_WMI_OTHER_GPU_TMP			= 1009,
        LENOVO_WMI_OTHER_GPU_TAC			= 1010,
        LENOVO_WMI_OTHER_GC_DB				= 1011,
        LENOVO_WMI_OTHER_IGPU_M				= 1012,
        LENOVO_WMI_OTHER_IB_AC				= 1013,
        LENOVO_WMI_OTHER_IB_UPD				= 1014,
        LENOVO_WMI_OTHER_FF_S				= 1015,
        LENOVO_WMI_OTHER_OD					= 1016,
        LENOVO_WMI_OTHER_GM_FNQ				= 1017,
        LENOVO_WMI_OTHER_NG_DDS				= 1018,
        LENOVO_WMI_OTHER_FL_ST				= 1019,
        LENOVO_WMI_OTHER_AMD_STT			= 1020,
        LENOVO_WMI_OTHER_SS_M				= 1021
    };

public:
    struct Other {

        struct StandardAttributesCapData01
        {
            StandardAttributesCapData01(const SysFsDriver::DescriptorType& descriptor,const QString& prefix):
                m_current_value(descriptor[prefix + "current_value"]),
                m_default_value(descriptor[prefix + "default_value"]),
                m_display_name(descriptor[prefix +  "display_name"]),
                m_max_value(descriptor[prefix + "max_value"]),
                m_min_value(descriptor[prefix + "min_value"]),
                m_scalar_increment(descriptor[prefix + "scalar_increment"]),
                m_steps(descriptor[prefix +  "steps"]),
                m_supported(descriptor[prefix +  "supported"]),
                m_type(descriptor[prefix +  "type"])
            {}


            const std::filesystem::path m_current_value;
            const std::filesystem::path m_default_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_max_value;
            const std::filesystem::path m_min_value;
            const std::filesystem::path m_scalar_increment;
            const std::filesystem::path m_steps;
            const std::filesystem::path m_supported;
            const std::filesystem::path m_type;
        };

        struct StandardAttributesCapData00
        {
            StandardAttributesCapData00(const SysFsDriver::DescriptorType& descriptor,const QString& prefix):
                m_current_value(descriptor[prefix + "current_value"]),
                m_default_value(descriptor[prefix + "default_value"]),
                m_display_name(descriptor[prefix +  "display_name"]),
                m_supported(descriptor[prefix +  "supported"]),
                m_type(descriptor[prefix +  "type"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_default_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
            const std::filesystem::path m_type;
        };

        struct CPU {
            CPU(const SysFsDriver::DescriptorType& descriptor) :
                m_cpu_clp_limit(descriptor,QString("cpu_clp_limit")),
                m_cpu_ltp_limit(descriptor,QString("cpu_ltp_limit")),
                m_cpu_pl1_tau(descriptor,QString("cpu_pl1_tau")),
                m_cpu_pp_limit(descriptor,QString("cpu_pp_limit")),
                m_cpu_stp_limit(descriptor,QString("cpu_stp_limit")),
                m_cpu_tmp_limit(descriptor,QString("cpu_tmp_limit")) {}


            const StandardAttributesCapData01 m_cpu_clp_limit;
            const StandardAttributesCapData01 m_cpu_ltp_limit;
            const StandardAttributesCapData01 m_cpu_pl1_tau;
            const StandardAttributesCapData01 m_cpu_pp_limit;
            const StandardAttributesCapData01 m_cpu_stp_limit;
            const StandardAttributesCapData01 m_cpu_tmp_limit;
        };

        struct GPU {
            GPU(const SysFsDriver::DescriptorType& descriptor) :
                m_gpu_configurable_tgp(descriptor,QString("gpu_configurable_tgp")),
                m_gpu_power_boost(descriptor,QString("gpu_power_boost")),
                m_gpu_temperature_limit(descriptor,QString("gpu_temperature_limit")),
                m_gpu_to_cpu_dynamic_boost(descriptor,QString("gpu_to_cpu_dynamic_boost")),
                m_gpu_total_onac(descriptor,QString("gpu_total_onac")){}


            const StandardAttributesCapData01 m_gpu_configurable_tgp;
            const StandardAttributesCapData01 m_gpu_power_boost;
            const StandardAttributesCapData01 m_gpu_temperature_limit;
            const StandardAttributesCapData01 m_gpu_to_cpu_dynamic_boost;
            const StandardAttributesCapData01 m_gpu_total_onac;
        };

        explicit Other(const SysFsDriver::DescriptorType& descriptor) :
            m_cpu(descriptor),
            m_gpu(descriptor),
            m_fan_full_speed(descriptor,"fan_full_speed"),
            m_god_mode_fnq_switchable(descriptor,"god_mode_fnq_switchable")
        {}


        const CPU                          m_cpu;
        const GPU                          m_gpu;
        const StandardAttributesCapData00  m_fan_full_speed;
        const StandardAttributesCapData00  m_god_mode_fnq_switchable;
    };
public:

    SysFsDriverLegionOther(QObject* parrent);

    ~SysFsDriverLegionOther() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

    void handleKernelEvent(const KernelEvent::Event &event) override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME   =  "legion_wmi_other";
    static constexpr const char* MODULE_NAME   =  LEGION_MODULE_NAME;
    static constexpr const char* DEPENDENCY[]  =  {"legion_wmi_cd01","legion_wmi_dd"};
};

}
