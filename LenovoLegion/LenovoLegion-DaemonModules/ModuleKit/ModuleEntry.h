// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Exported entry points of a daemon module (.so) made of a class with constructor (IModuleHost&):
 *
 *   LEGION_KIT_MODULE(ModuleBattery,"Battery","1.0",legion::messages::DataType::BATTERY,"BatteryData")
 *
 * settingsGroup may be nullptr (module name is used).
 */

#include <ModuleApi.h>

#define LEGION_KIT_MODULE(ModuleClass,moduleName,moduleVersion,moduleLegacyDataType,moduleSettingsGroup)         \
    LEGION_MODULE_DECLARE_ENTRY_POINTS                                                                          \
                                                                                                                \
    const LenovoLegionDaemon::Module::Info* legion_module_info()                                                \
    {                                                                                                           \
        static const LenovoLegionDaemon::Module::Info info {                                                    \
            .abiVersion       = LenovoLegionDaemon::Module::ABI_VERSION,                                        \
            .name             = moduleName,                                                                     \
            .version          = moduleVersion,                                                                  \
            .legacyDataType   = static_cast<int>(moduleLegacyDataType),                                         \
            .settingsGroup    = moduleSettingsGroup,                                                            \
            .requiredServices = nullptr                                                                         \
        };                                                                                                      \
        return &info;                                                                                           \
    }                                                                                                           \
                                                                                                                \
    LenovoLegionDaemon::Module::IModule* legion_module_create(LenovoLegionDaemon::Module::IModuleHost& host)    \
    {                                                                                                           \
        return new ModuleClass(host);                                                                           \
    }                                                                                                           \
                                                                                                                \
    void legion_module_destroy(LenovoLegionDaemon::Module::IModule* module)                                     \
    {                                                                                                           \
        delete module;                                                                                          \
    }
