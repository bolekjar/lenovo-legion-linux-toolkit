// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ApplicationModulesHandler.h>

#include <QFileInfo>
#include <QString>

namespace LenovoLegionDaemon {

class DataProviderManager;
class SysFsDriverManager;

/*
 * Loads daemon modules (.so) from the modules directory and registers them as data providers.
 *
 * The daemon runs as root, so only trusted files are loaded: the modules directory and every module
 * must be owned by root or by the owner of the daemon executable (development build) and must not be
 * writable by group or others. Failure of one module is logged and the module is skipped.
 */
class ModuleLoader
{
public:

    /*
     * Returns number of registered modules
     */
    static int loadModules(bj::framework::ApplicationModulesHandler& modules,SysFsDriverManager* sysFsDriverManager,DataProviderManager* dataProviderManager);

    /*
     * Security check of a module file or of the modules directory
     */
    static bool isTrusted(const QFileInfo& file,bool directory,QString& reason);
};

}
