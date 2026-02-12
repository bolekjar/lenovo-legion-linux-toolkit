// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QRegularExpression>
#include <QDir>
#include <QString>
#include <QLibrary>
#include <QList>

#include <functional>
#include <vector>


namespace bj { namespace framework {

class ApplicationModulesLoader
{

private:

    template <
        size_t   pos,
        typename ... Signatures
        >
    struct ModuleImpl;

    template <
        typename ... Signatures
        >
    struct ModuleImpl<0,Signatures ...> {
        using FactoryFn = std::function<std::tuple_element_t<0,std::tuple<Signatures...>>>;

        FactoryFn m_factoryFnc;
    };


    template <
        size_t   pos,
        typename ... Signatures
        >
    struct ModuleImpl : public ModuleImpl<pos - 1,Signatures ...>
    {
        using FactoryFn = std::function<std::tuple_element_t<pos,std::tuple<Signatures...>>>;

        FactoryFn m_factoryFnc;
    };


public:

    template <
        typename ... Signatures
        >
    using Module  = ModuleImpl<std::tuple_size_v<std::tuple<Signatures ...>> - 1,Signatures ...>;

    template <
        typename ... Signatures
        >
    using Modules = QList<Module<Signatures...>>;

    template <
        size_t value,
        typename ... Signatures
        >
    using ModuleItemGet  = ModuleImpl<value,Signatures ...>;

private:

    template<size_t c,typename ... Types>
    struct ForEach
    {
        using Tuple       = std::tuple<Types...>;
        using ModuleImpl  = ApplicationModulesLoader::ModuleImpl<c,Types ...>;
        using Signature   = std::tuple_element_t<c,Tuple>;


        static bool add_module(ModuleImpl& moduleAdd,QLibrary& module,const std::vector<QString>& symbols) {
            typename ModuleImpl::FactoryFn pointer(reinterpret_cast<typename std::add_pointer<Signature>::type>(module.resolve(symbols.at(c).toStdString().c_str())));

            if(pointer != nullptr)
            {
                moduleAdd.m_factoryFnc = pointer;
                return ForEach<c - 1,Types ...>::add_module(moduleAdd,module,symbols);;
            }

            return false;
        }
    };

    template<typename ... Types>
    struct ForEach<0,Types ...>
    {
        using Tuple         = std::tuple<Types...>;
        using ModuleImpl    = ApplicationModulesLoader::ModuleImpl<0,Types ...>;
        using Signature     = std::tuple_element_t<0,Tuple>;

        static bool add_module(ModuleImpl& moduleAdd,QLibrary& module,const std::vector<QString>& symbols)
        {
            typename ModuleImpl::FactoryFn pointer(reinterpret_cast<typename std::add_pointer<Signature>::type>(module.resolve(symbols.at(0).toStdString().c_str())));

            if(pointer != nullptr)
            {
                moduleAdd.m_factoryFnc = pointer;
                return true;
            }

            return false;
        }
    };

public:

    ApplicationModulesLoader(const QRegularExpression& moduleSulfix = QRegularExpression(".dll|so$") );
    ApplicationModulesLoader(const QDir& modulePath,const QRegularExpression& moduleSulfix = QRegularExpression(".dll|so$") );

    ApplicationModulesLoader(const ApplicationModulesLoader&)   =  delete;
    ApplicationModulesLoader(const ApplicationModulesLoader &&) =  delete;

    ApplicationModulesLoader& operator =(const ApplicationModulesLoader&)   =  delete;
    ApplicationModulesLoader& operator =(const ApplicationModulesLoader &&) =  delete;


    /*
     * Setters
     */
    void setSuffix(const QRegularExpression& moduleSulfix);
    void setPath(const QDir& modulePath);

    /*
     * Getters
     */
    const QDir& path()                 const;
    const QRegularExpression& suffix() const;


    /*
     * Check if path to modules exists
     */
    bool existsPath() const;

    /*
     * Load modules from file into memory
     */
    int  load();

    /*
     * Unload modules from memory, Reload modules into memory from disk
     */
    int  reload(const std::chrono::milliseconds& maxWaitForUnloading = std::chrono::milliseconds(0));
    bool unload(const std::chrono::milliseconds& maxWaitForUnloading = std::chrono::milliseconds(0));

template<typename ... Signatures>
    auto getModules(const std::vector<QString>& symbols);

private:

    QRegularExpression m_moduleSulfix;
    QDir               m_modulesPath;

    std::list<QLibrary> m_modules;
};


template<typename ... Signatures>
auto ApplicationModulesLoader::getModules(const std::vector<QString>& symbols)
{
    using Modules           = ApplicationModulesLoader::Modules<Signatures ...>;

    Modules modules;

    for(auto& module : m_modules)
    {
        typename Modules::Type addModule;
        if(ForEach<std::tuple_size_v<std::tuple<Signatures...>> - 1,Signatures ...>::add_module(addModule,module,symbols))
        {

            modules.push_back(addModule);
        }
    }

    return modules;
}


}}
