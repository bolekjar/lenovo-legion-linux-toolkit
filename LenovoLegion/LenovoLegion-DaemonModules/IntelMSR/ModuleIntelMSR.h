// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Daemon module IntelMSR - CPU voltage offsets (kernel module legion_intel_msr, class legion-intel-msr,
 * /sys/class/legion-intel-msr/legion-intel-msr-0/attributes/<plane>_offset/).
 * Pilot module, statically linked into the daemon (see LenovoLegion-Daemon/ModulesDesign.md).
 */

#include <ModuleApi.h>

#include <filesystem>

namespace LenovoLegionDaemon::Modules::IntelMSR {

/*
 * Entry points (for the .so version wrapped by LEGION_MODULE_DECLARE_ENTRY_POINTS)
 */
const Module::Info* info();
Module::IModule*    create(Module::IModuleHost& host);
void                destroy(Module::IModule* module);

/*
 * Same as create(), sysfs directory of the device can be changed (tests)
 */
Module::IModule*    create(Module::IModuleHost& host,const std::filesystem::path& sysfsPath);

}
