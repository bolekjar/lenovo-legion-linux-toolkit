// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Interface of one module (one feature, e.g. IntelMSR, including its internal sysfs drivers).
 *
 * Rules:
 *  - all methods are called from the daemon event loop thread,
 *  - methods may throw, the framework catches every call:
 *      init()            -> module is FAILED, other modules run,
 *      get()/set()       -> ERROR_RESPONSE to the client, the connection stays open,
 *      other methods     -> logged,
 *  - invalid values in set() should be logged and ignored, not thrown (the GUI re-reads the values after set),
 *  - the module must not keep references to the host after clean().
 */

#include "ModuleTypes.h"

namespace LenovoLegionDaemon::Module {

class IModuleHost;

class LEGION_MODULE_VISIBLE IModule
{
public:

    struct SettingsPolicy
    {
        bool persistent = false;  // module saves and restores settings
        int  order      = 100;    // order of restoring on start, lower first (PowerProfile = 0)
        bool customOnly = false;  // restore only when CUSTOM power profile is saved (FanCurve, CPUPower, GPUPower)
    };

public:

    virtual ~IModule() = default;


    /*
     * Lifecycle
     */

    /* Detect hardware, register kernel event filters and timers. Throw if the module can not run. */
    virtual void init()  = 0;

    /* Release everything acquired in init() (timers are removed by the framework) */
    virtual void clean() = 0;

    /* Hardware is available (e.g. kernel module is loaded), unavailable module answers get() with empty data */
    virtual bool isAvailable() const = 0;


    /*
     * Data, request and response are messages defined by the module (protobuf), the framework does not know them
     */

    /* GET_DATA_REQUEST, request may be empty */
    virtual QByteArray get(const QByteArray& request) = 0;

    /* SET_DATA_REQUEST */
    virtual QByteArray set(const QByteArray& request) = 0;


    /*
     * Kernel events of the subsystems registered by IModuleHost::addKernelEventFilter
     */
    virtual void onKernelEvent(const KernelEvent&) {}


    /*
     * Settings
     */
    virtual SettingsPolicy settingsPolicy() const        { return {}; }

    /* Read saved settings and apply them to the hardware */
    virtual void loadSettings(const ISettingsStorage&)   {}

    /* Read current state of the hardware and save it */
    virtual void saveSettings(ISettingsStorage&)         {}
};

}
