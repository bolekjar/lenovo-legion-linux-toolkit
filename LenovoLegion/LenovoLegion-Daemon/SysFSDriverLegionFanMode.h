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

class SysFSDriverLegionFanMode : public SysFsDriver
{
public:
    struct FanMode {

        struct FanCurve {

            FanCurve(const SysFsDriver::DescriptorType& descriptor) :
                m_cpu_fan_default(descriptor["cpu_fan_default"]),
                m_cpusen_fan_default(descriptor["cpusen_fan_default"]),
                m_cpusen_sensor_default(descriptor["cpusen_sensor_default"]),
                m_cpu_sensor_default(descriptor["cpu_sensor_default"]),
                m_current_value(descriptor["current_value"]),
                m_display_name(descriptor["display_name"]),
                m_sys_fan_default(descriptor["sys_fan_default"]),
                m_sys_sensor_default(descriptor["sys_sensor_default"]),
                m_gpu_fan_default(descriptor["gpu_fan_default"])    ,
                m_gpu_sensor_default(descriptor["gpu_sensor_default"])
            {}

            const std::filesystem::path m_cpu_fan_default;
            const std::filesystem::path m_cpusen_fan_default;
            const std::filesystem::path m_cpusen_sensor_default;
            const std::filesystem::path m_cpu_sensor_default;
            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_sys_fan_default;
            const std::filesystem::path m_sys_sensor_default;
            const std::filesystem::path m_gpu_fan_default;
            const std::filesystem::path m_gpu_sensor_default;
        };


        explicit FanMode(const SysFsDriver::DescriptorType& descriptor) :
            m_fanCurve(descriptor)
        {}


        const FanCurve         m_fanCurve;
    };
public:

    SysFSDriverLegionFanMode(QObject * parrent);

    ~SysFSDriverLegionFanMode() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;


    void handleKernelEvent(const KernelEvent::Event &event) override;


public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME   =  "legion_wmi_fan_method";
    static constexpr const char* MODULE_NAME   =  LEGION_MODULE_NAME;
    static constexpr const char* DEPENDENCY[]  =  {"legion_wmi_ftable"};

};


}
