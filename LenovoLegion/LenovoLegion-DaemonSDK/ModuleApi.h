// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Entry points of a module. Statically linked module (pilot) provides the same functions
 * without extern "C" and is registered by the framework directly.
 */

#include "IModule.h"
#include "IModuleHost.h"

#include <cstdint>

namespace LenovoLegionDaemon::Module {

/*
 * Increase on every incompatible change of the SDK (headers in this directory)
 */
constexpr uint32_t ABI_VERSION = 1;

/*
 * Legacy data type is not used, the framework assigns one (dynamic data types, LIST_MODULES)
 */
constexpr int NO_LEGACY_DATA_TYPE = -1;

struct Info
{
    uint32_t            abiVersion;         // must be ABI_VERSION of the framework
    const char*         name;               // unique module name, e.g. "IntelMSR"
    const char*         version;            // module version (informative)

    /*
     * Transitional compatibility with the current GUI and settings
     */
    int                 legacyDataType;     // data type from MessageRegistry.proto or NO_LEGACY_DATA_TYPE
    const char*         settingsGroup;      // group in the daemon settings file, e.g. "IntelMSRData" (nullptr = name)

    const char* const*  requiredServices;   // nullptr terminated list of system services, nullptr = none
};

/*
 * Factory functions
 */
using InfoFn    = const Info* ();
using CreateFn  = IModule*    (IModuleHost& host);
using DestroyFn = void        (IModule* module);

/*
 * Names of the exported symbols (ApplicationModulesLoader::getModules)
 */
constexpr const char* INFO_SYMBOL    = "legion_module_info";
constexpr const char* CREATE_SYMBOL  = "legion_module_create";
constexpr const char* DESTROY_SYMBOL = "legion_module_destroy";

}

/*
 * Exported entry points of a module (.so)
 *
 *  legion_module_info    - called first, the framework checks ABI version and name before creating anything
 *  legion_module_create  - host is valid until legion_module_destroy
 *  legion_module_destroy - module is deleted in the same library where it was created
 */
#define LEGION_MODULE_EXPORT extern "C" __attribute__((visibility("default")))

#define LEGION_MODULE_DECLARE_ENTRY_POINTS                                                                              \
    LEGION_MODULE_EXPORT const LenovoLegionDaemon::Module::Info* legion_module_info();                                   \
    LEGION_MODULE_EXPORT LenovoLegionDaemon::Module::IModule*    legion_module_create(LenovoLegionDaemon::Module::IModuleHost& host); \
    LEGION_MODULE_EXPORT void                                    legion_module_destroy(LenovoLegionDaemon::Module::IModule* module);
