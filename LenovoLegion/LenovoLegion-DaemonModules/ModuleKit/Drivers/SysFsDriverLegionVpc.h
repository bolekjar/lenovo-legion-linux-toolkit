// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include "../SysFsDriver.h"
#include "SysFsDriverLegion.h"

namespace LenovoLegionDaemon::Kit {

/*
 * Features of the ACPI device VPC2004 (lenovo_legion legion-acpi-vpc.c): conservation mode, USB charging,
 * Fn lock, camera power. Every feature is optional (directory of the feature exists, its supported is 1).
 */
class SysFsDriverLegionVpc : public SysFsDriver
{
public:

    struct Vpc {

        struct Feature {

            Feature(const SysFsDriver::DescriptorType& descriptor,const QString& name) :
                m_current_value(descriptor.value(name + "_current_value")),
                m_supported(descriptor.value(name + "_supported"))
            {}

            bool exists() const { return !m_current_value.empty() && !m_supported.empty(); }

            const std::filesystem::path m_current_value;
            const std::filesystem::path m_supported;
        };

        explicit Vpc(const SysFsDriver::DescriptorType& descriptor) :
            m_conservationMode(descriptor,"conservation_mode"),
            m_usbCharging(descriptor,"usb_charging"),
            m_fnLock(descriptor,"fn_lock"),
            m_cameraPower(descriptor,"camera_power")
        {}

        const Feature m_conservationMode;
        const Feature m_usbCharging;
        const Feature m_fnLock;
        const Feature m_cameraPower;
    };

    /*
     * EVENT_TYPE of the uevents of the class device (legion-common.h, order of the features)
     */
    enum class EventType : int {
        CONSERVATION_MODE = 3000,
        USB_CHARGING      = 3001,
        FN_LOCK           = 3002,
        CAMERA_POWER      = 3003
    };

public:

    SysFsDriverLegionVpc();

    ~SysFsDriverLegionVpc() override = default;

    virtual void init() override;

    void handleKernelEvent(const KernelEvent::Event &event) override;

public:

    static constexpr const char* DRIVER_NAME =  "legion_acpi_vpc";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;

    /*
     * Class device of the features (uevents add, remove, change)
     */
    static constexpr const char* SUBSYSTEM   =  "legion-firmware-attributes";
    static constexpr const char* SYS_NAME    =  "legion-vpc-0";
};

}
