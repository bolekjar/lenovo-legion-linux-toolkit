// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include "../SysFsDriver.h"
#include "SysFsDriverLegion.h"

#include <optional>



namespace LenovoLegionDaemon::Kit {

class SysFsDriverLegionEvents : public SysFsDriver
{
public:

    enum LegionVmiEventType  : int
    {
        LENOVO_WMI_EVENT_UTILITY            = 0,
        LEGION_WMI_EVENT_THERMAL_MODE 		= 1,
        LEGION_WMI_EVENT_POWER_CHARGE_MODE 	= 2,
        LEGION_WMI_EVENT_SMART_FAN_MODE     = 3,
        LEGION_WMI_EVENT_KEYLOCK_STATUS     = 4
    };

public:

    SysFsDriverLegionEvents();

    ~SysFsDriverLegionEvents() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

    virtual void handleKernelEvent(const SysFsDriver::KernelEvent::Event& event) override;

    /*
     * New thermal mode of a thermal mode event (Fn+Q or set by a client), empty for other events
     */
    static std::optional<int> thermalMode(const SysFsDriver::SubsystemEvent& event)
    {
        bool ok = false;

        if(event.m_driverName != DRIVER_NAME || event.m_action != SysFsDriver::SubsystemEvent::Action::CHANGED ||
           event.m_DriverSpecificEventType.toInt(&ok) != LEGION_WMI_EVENT_THERMAL_MODE || !ok)
        {
            return {};
        }

        const int mode = event.m_DriverSpecificEventValue.toInt(&ok);

        return ok ? std::optional<int>(mode) : std::nullopt;
    }

    /*
     * Thermal mode custom (PowerProfile POWER_PROFILE_CUSTOM)
     */
    static constexpr int THERMAL_MODE_CUSTOM = 255;
public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "legion_wmi_events";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;

private:

    static constexpr std::string_view EVENT_TYPE    = "EVENT_TYPE";
    static constexpr std::string_view EVENT_VALUE   = "EVENT_VALUE";
};

}
