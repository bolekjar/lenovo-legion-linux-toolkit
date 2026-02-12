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

class SysFSDriverLegionGameZone : public SysFsDriver
{
public:

    struct GameZone {

        struct SmartFan {

            SmartFan(const SysFsDriver::DescriptorType& descriptor) :
                m_current_value(descriptor["smart_fan_current_value"]),
                m_display_name(descriptor["smart_fan_display_name"]),
                m_supported(descriptor["smart_fan_supported"]),
                m_extreme_supported(descriptor["smart_fan_extreme_supported"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
            const std::filesystem::path m_extreme_supported;
        };

        struct DisableTP {

            DisableTP(const SysFsDriver::DescriptorType& descriptor) :
                m_current_value(descriptor["disable_tp_current_value"]),
                m_display_name(descriptor["disable_tp_display_name"]),
                m_supported(descriptor["disable_tp_supported"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
        };

        struct DisableWinKey {

            DisableWinKey(const SysFsDriver::DescriptorType& descriptor) :
                m_current_value(descriptor["disable_win_key_current_value"]),
                m_display_name(descriptor["disable_win_key_display_name"]),
                m_supported(descriptor["disable_win_key_supported"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
        };

        struct Gsync {

            Gsync(const SysFsDriver::DescriptorType& descriptor) :
                m_current_value(descriptor["gsync_current_value"]),
                m_display_name(descriptor["gsync_display_name"]),
                m_supported(descriptor["gsync_supported"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
        };

        struct IGPUMode {
            IGPUMode(const SysFsDriver::DescriptorType& descriptor) :
                m_current_value(descriptor["igpu_mode_current_value"]),
                m_display_name(descriptor["igpu_mode_display_name"]),
                m_supported(descriptor["igpu_mode_supported"])
            {}

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_display_name;
            const std::filesystem::path m_supported;
        };

        struct Other {

            Other(const SysFsDriver::DescriptorType& descriptor) :
                get_power_charge_mode(descriptor["other_get_power_charge_mode"]),
                get_thermal_mode(descriptor["other_get_thermal_mode"])
            {}

            const std::filesystem::path  get_power_charge_mode;
            const std::filesystem::path  get_thermal_mode;
        };

        explicit GameZone(const SysFsDriver::DescriptorType& descriptor) :
            m_smartFan(descriptor),
            m_other(descriptor),
            m_disableTP(descriptor),
            m_disableWinKey(descriptor),
            m_gsync(descriptor),
            m_igpuMode(descriptor)
        {}

        const SmartFan      m_smartFan;
        const Other         m_other;
        const DisableTP     m_disableTP;
        const DisableWinKey m_disableWinKey;
        const Gsync         m_gsync;
        const IGPUMode      m_igpuMode;
    };

public:

    SysFSDriverLegionGameZone(QObject * parrent);

    ~SysFSDriverLegionGameZone() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

    void handleKernelEvent(const KernelEvent::Event &event) override;

public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "legion_wmi_gamezone";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;


};

}
