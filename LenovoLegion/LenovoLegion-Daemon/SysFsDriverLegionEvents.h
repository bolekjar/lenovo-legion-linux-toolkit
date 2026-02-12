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

    SysFsDriverLegionEvents(QObject * parrent);

    ~SysFsDriverLegionEvents() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;

    virtual void handleKernelEvent(const SysFsDriver::KernelEvent::Event& event) override;
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
