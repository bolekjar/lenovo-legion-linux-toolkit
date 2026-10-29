// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ApplicationModulesLoader.h>

namespace bj { namespace framework {

/*
 * Handling of modules
 */
struct ApplicationModulesHandler {

    ApplicationModulesHandler(ApplicationModulesLoader& modules);

    /*
     * Only these operations are permited for application
     */
    int load();

    bool unload(const std::chrono::milliseconds& maxWaitForUnloading);

    int reload(const std::chrono::milliseconds& maxWaitForUnloading);

    template<typename ... Signatures>
    auto getModules(const std::vector<QString>& symbols);

    /*
     * Filter of files before loading (e.g. security check), must be set before load()
     */
    void setFileFilter(ApplicationModulesLoader::FileFilter filter);

    /*
     * Files which were not loaded and libraries without the requested symbols
     */
    const std::vector<ApplicationModulesLoader::Error>& errors() const;

    const QDir& path() const;

private:

    ApplicationModulesLoader& m_modules;
};

template<typename ... Signatures>
auto ApplicationModulesHandler::getModules(const std::vector<QString> &symbols)
{
    return m_modules.getModules<Signatures ...>(symbols);
}


}}
