// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ModuleLoader.h"
#include "ModuleDataProviderAdapter.h"

#include "../DataProviderManager.h"

#include <Core/LoggerHolder.h>

#include <QCoreApplication>
#include <QSet>

#include <sys/stat.h>

namespace LenovoLegionDaemon {

namespace {

/*
 * Owner of the daemon executable, 0 (root) for the installed daemon, developer for a development build
 */
uid_t executableOwner()
{
    struct stat st {};

    if(stat(QCoreApplication::applicationFilePath().toLocal8Bit().constData(),&st) != 0)
    {
        return 0;
    }

    return st.st_uid;
}

void logLoaderErrors(const bj::framework::ApplicationModulesHandler& modules,size_t from)
{
    for(size_t i = from; i < modules.errors().size(); ++i)
    {
        LOG_E(QString("Module ").append(modules.errors().at(i).m_file).append(" is not loaded: ").append(modules.errors().at(i).m_reason));
    }
}

}

bool ModuleLoader::isTrusted(const QFileInfo &file, bool directory, QString &reason)
{
    struct stat st {};

    /*
     * lstat - symlinks are not accepted
     */
    if(lstat(file.absoluteFilePath().toLocal8Bit().constData(),&st) != 0)
    {
        reason = QString("Unable to stat: ").append(strerror(errno));
        return false;
    }

    if(directory ? !S_ISDIR(st.st_mode) : !S_ISREG(st.st_mode))
    {
        reason = directory ? "Not a directory" : "Not a regular file";
        return false;
    }

    if(st.st_uid != 0 && st.st_uid != executableOwner())
    {
        reason = QString("Owner uid=").append(QString::number(st.st_uid)).append(" is not root or the owner of the daemon executable");
        return false;
    }

    if((st.st_mode & (S_IWGRP | S_IWOTH)) != 0)
    {
        reason = "Writable by group or others";
        return false;
    }

    return true;
}

int ModuleLoader::loadModules(bj::framework::ApplicationModulesHandler &modules, SysFsDriverManager *sysFsDriverManager, DataProviderManager *dataProviderManager)
{
    QString reason;

    LOG_I(QString("Loading daemon modules from ").append(modules.path().absolutePath()));

    if(!isTrusted(QFileInfo(modules.path().absolutePath()),true,reason))
    {
        LOG_E(QString("Modules directory ").append(modules.path().absolutePath()).append(" is not trusted (").append(reason).append("), no module is loaded !"));
        return 0;
    }

    modules.setFileFilter([](const QFileInfo& file,QString& reason) {
        return isTrusted(file,false,reason);
    });

    if(modules.load() < 0)
    {
        LOG_E(QString("Unable to load modules from ").append(modules.path().absolutePath()));
        return 0;
    }

    logLoaderErrors(modules,0);

    /*
     * Libraries with the module entry points
     */
    const size_t resolveErrorsFrom = modules.errors().size();

    auto entryPoints = modules.getModules<Module::InfoFn,Module::CreateFn,Module::DestroyFn>({ Module::INFO_SYMBOL, Module::CREATE_SYMBOL, Module::DESTROY_SYMBOL });

    logLoaderErrors(modules,resolveErrorsFrom);

    using Loader = bj::framework::ApplicationModulesLoader;

    QSet<QString> names;
    int           registered = 0;

    for(auto& entry : entryPoints)
    {
        ModuleDataProviderAdapter::Definition definition {
            .m_info    = static_cast<Loader::ModuleItemGet<0,Module::InfoFn,Module::CreateFn,Module::DestroyFn>&>(entry).m_factoryFnc,
            .m_create  = static_cast<Loader::ModuleItemGet<1,Module::InfoFn,Module::CreateFn,Module::DestroyFn>&>(entry).m_factoryFnc,
            .m_destroy = static_cast<Loader::ModuleItemGet<2,Module::InfoFn,Module::CreateFn,Module::DestroyFn>&>(entry).m_factoryFnc
        };

        ModuleDataProviderAdapter* adapter = nullptr;

        try {
            adapter = new ModuleDataProviderAdapter(definition,sysFsDriverManager,dataProviderManager);

            if(names.contains(adapter->info().name))
            {
                THROW_EXCEPTION(ModuleDataProviderAdapter::exception_T,ModuleDataProviderAdapter::ERROR_CODES::INVALID_MODULE,QString("Module ").append(adapter->info().name).append(" is already loaded !").toStdString().c_str());
            }

            dataProviderManager->addDataProvider(adapter);

            names.insert(adapter->info().name);
            ++registered;

            LOG_I(QString("Module ").append(adapter->info().name).append(" version=").append(adapter->info().version).append(" loaded"));
        }
        catch(const bj::framework::exception::Exception& ex)
        {
            LOG_E(QString("Module is not loaded: ").append(bj::framework::exception::ExceptionBuilder::print(ex).c_str()));
            delete adapter;
        }
        catch(const std::exception& ex)
        {
            LOG_E(QString("Module is not loaded: ").append(ex.what()));
            delete adapter;
        }
        catch(...)
        {
            LOG_E("Module is not loaded: unknown error !");
            delete adapter;
        }
    }

    LOG_I(QString("Daemon modules loaded: ").append(QString::number(registered)));

    return registered;
}

}
