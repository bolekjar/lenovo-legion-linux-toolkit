// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ApplicationModulesLoader.h"

#include <Core/Logger.h>

#include <QRegularExpression>
#include <QLibrary>

#include <chrono>

namespace bj { namespace framework {


ApplicationModulesLoader::ApplicationModulesLoader(const QRegularExpression &moduleSulfix) :
   m_moduleSulfix(moduleSulfix),
    m_modulesPath(QDir::homePath().append(QDir::separator()).append("modules").append(QDir::separator()))
{}

void ApplicationModulesLoader::setSuffix(const QRegularExpression &moduleSulfix)
{
    m_moduleSulfix = moduleSulfix;
}

void ApplicationModulesLoader::setPath(const QDir &modulePath)
{
    m_modulesPath = modulePath;
}

const QDir& ApplicationModulesLoader::path() const
{
    return m_modulesPath;
}

const QRegularExpression& ApplicationModulesLoader::suffix() const
{
    return m_moduleSulfix;
}

ApplicationModulesLoader::ApplicationModulesLoader(const QDir& modulePath, const QRegularExpression &moduleSulfix) :
    m_moduleSulfix(moduleSulfix),
    m_modulesPath(modulePath)
{}

bool ApplicationModulesLoader::existsPath() const
{
    return m_modulesPath.exists();
}

int ApplicationModulesLoader::load()
{
    if(!existsPath())
    {
        return -1;
    }

    m_modules.clear();
    m_modulesPath.refresh();

    for (auto& file : m_modulesPath.entryInfoList())
    {
        if(file.absoluteFilePath().contains(m_moduleSulfix))
        {
            m_modules.emplace_back(file.absoluteFilePath(),1);

            if(!m_modules.back().load())
            {
                m_modules.pop_back();
            }
        }
    }


    return m_modules.size();
}

int ApplicationModulesLoader::reload(const std::chrono::milliseconds &maxWaitForUnloading)
{
    bool ret = unload(maxWaitForUnloading);

    if(ret == true)
    {
        return load();
    }

    return -1;
}

bool ApplicationModulesLoader::unload(const std::chrono::milliseconds& maxWaitForUnloading)
{
    bool ret = true;

    for (auto& module : m_modules)
    {
        ret = module.unload();
    }

    if(ret == false)
    {
        return ret;
    }


    auto t1 = std::chrono::system_clock::now();
    while(true)
    {
        bool stop = true;
        for (auto& module : m_modules)
        {
            module.unload();
            if(module.isLoaded())
            {
                stop = false;
            }
        }

        if(stop)
        {
            break;
        }

        if((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()) -
            std::chrono::duration_cast<std::chrono::milliseconds>(t1.time_since_epoch())
           ) > maxWaitForUnloading
          )
        {
            ret = false;
            break;
        }
    }

    return ret;
}

}}
