// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Core/ApplicationModulesHandler.h>

namespace bj { namespace framework {


ApplicationModulesHandler::ApplicationModulesHandler(ApplicationModulesLoader &modules) :
    m_modules(modules)
{}

int ApplicationModulesHandler::load()
{
    return m_modules.load();
}

bool ApplicationModulesHandler::unload(const std::chrono::milliseconds &maxWaitForUnloading)
{
    return m_modules.unload(maxWaitForUnloading);
}

int ApplicationModulesHandler::reload(const std::chrono::milliseconds &maxWaitForUnloading)
{
    return m_modules.reload(maxWaitForUnloading);
}


}}
