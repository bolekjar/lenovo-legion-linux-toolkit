// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../DataProvider.h"

#include <ModuleApi.h>

#include <functional>

namespace LenovoLegionDaemon {

class ModuleHost;
class SysFsDriverManager;

/*
 * Connects a daemon module (IModule) to the current framework as a DataProvider:
 * the protocol (dataType = Info::legacyDataType), settings and notifications stay unchanged.
 *
 *  - exception from init() -> module is FAILED, get/set throw (client gets ERROR_RESPONSE), other modules run
 *  - exceptions from get/set go to ProtocolProcessor::processRequest (ERROR_RESPONSE)
 *  - exceptions from settings and clean are only logged
 */
class ModuleDataProviderAdapter : public DataProvider
{
public:

    DEFINE_EXCEPTION(ModuleDataProviderAdapter);

    enum ERROR_CODES : int {
        INVALID_MODULE  = -1
    };

    /*
     * Entry points of a module
     */
    struct Definition
    {
        std::function<Module::InfoFn>     m_info;
        std::function<Module::CreateFn>   m_create;
        std::function<Module::DestroyFn>  m_destroy;
    };

public:

    ModuleDataProviderAdapter(const Definition& definition,SysFsDriverManager* sysFsDriverManager,QObject* parent);

    const Module::Info& info() const;

    /*
     * Settings policy of the module, not persistent when the module is not running
     */
    Module::IModule::SettingsPolicy settingsPolicy() const;
    ~ModuleDataProviderAdapter() override;

    QByteArray serializeAndGetData()                         override;
    QByteArray serializeAndGetData(const QByteArray& data)   override;
    QByteArray deserializeAndSetData(const QByteArray& data) override;

    void init()  override;
    void clean() override;

    void loadSettings() override;
    void saveSettings() override;

    /*
     * Settings in the groups with the prefix (saved profile, SettingsProfiles::prefix)
     */
    void loadSettings(const QString& prefix);
    void saveSettings(const QString& prefix);

private:

    enum class State
    {
        CREATED,
        RUNNING,
        FAILED
    };

    /*
     * Check the module info, returns legacy data type
     */
    static quint8 checkInfo(const Module::Info* info);

    void    checkRunning() const;
    QString settingsGroup() const;

private:

    const Module::Info*              m_info;
    std::function<Module::DestroyFn> m_destroy;
    ModuleHost*         m_host;
    Module::IModule*    m_module;
    State               m_state;
};

}
