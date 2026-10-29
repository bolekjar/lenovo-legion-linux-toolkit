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


static bool acceptAllFiles(const QFileInfo&,QString&)
{
    return true;
}

QRegularExpression ApplicationModulesLoader::defaultSuffix()
{
    return QRegularExpression("\\.(so|dll)$");
}

ApplicationModulesLoader::ApplicationModulesLoader(const QRegularExpression &moduleSulfix) :
   m_moduleSulfix(moduleSulfix),
    m_modulesPath(QDir::homePath().append(QDir::separator()).append("modules").append(QDir::separator())),
    m_fileFilter(acceptAllFiles)
{}

void ApplicationModulesLoader::setFileFilter(FileFilter filter)
{
    m_fileFilter = filter ? std::move(filter) : FileFilter(acceptAllFiles);
}

const std::vector<ApplicationModulesLoader::Error> &ApplicationModulesLoader::errors() const
{
    return m_errors;
}

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
    m_modulesPath(modulePath),
    m_fileFilter(acceptAllFiles)
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
    m_errors.clear();
    m_modulesPath.refresh();

    /*
     * Only files (also symlinks to files, the filter decides), the suffix is matched against the file name
     */
    for (auto& file : m_modulesPath.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Name))
    {
        if(!file.fileName().contains(m_moduleSulfix))
        {
            continue;
        }

        QString reason;

        if(!m_fileFilter(file,reason))
        {
            m_errors.push_back({file.absoluteFilePath(),reason.isEmpty() ? QString("Rejected by the file filter") : reason});
            continue;
        }

        m_modules.emplace_back(file.absoluteFilePath(),1);

        if(!m_modules.back().load())
        {
            m_errors.push_back({file.absoluteFilePath(),m_modules.back().errorString()});
            m_modules.pop_back();
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
