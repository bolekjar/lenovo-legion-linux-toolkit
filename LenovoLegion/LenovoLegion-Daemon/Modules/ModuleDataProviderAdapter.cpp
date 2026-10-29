// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ModuleDataProviderAdapter.h"
#include "ModuleHost.h"
#include "ModuleSettingsStorage.h"

#include <Core/LoggerHolder.h>

#include <limits>

namespace LenovoLegionDaemon {

ModuleDataProviderAdapter::ModuleDataProviderAdapter(const Definition &definition, SysFsDriverManager *sysFsDriverManager, QObject *parent) :
    DataProvider(parent,checkInfo(definition.m_info != nullptr ? definition.m_info() : nullptr)),
    m_info(definition.m_info()),
    m_destroy(definition.m_destroy),
    m_host(new ModuleHost(m_info->name,sysFsDriverManager,this)),
    m_module(nullptr),
    m_state(State::CREATED)
{
    if(definition.m_create == nullptr || definition.m_destroy == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MODULE,QString("Module ").append(m_info->name).append(" has no create/destroy function !").toStdString().c_str());
    }

    m_module = definition.m_create(*m_host);

    if(m_module == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MODULE,QString("Module ").append(m_info->name).append(" was not created !").toStdString().c_str());
    }

    m_host->setModule(m_module);

    connect(m_host,&ModuleHost::notification,this,&DataProvider::notification);

    LOG_D(QString("Module ").append(m_info->name).append(" version=").append(m_info->version).append(" created, dataType=").append(QString::number(m_dataType)));
}

const Module::Info &ModuleDataProviderAdapter::info() const
{
    return *m_info;
}

Module::IModule::SettingsPolicy ModuleDataProviderAdapter::settingsPolicy() const
{
    Module::IModule::SettingsPolicy policy {};

    if(m_state == State::RUNNING)
    {
        m_host->callModule("settings policy",[this,&policy] { policy = m_module->settingsPolicy(); });
    }

    if(m_state != State::RUNNING)
    {
        policy.persistent = false;
    }

    return policy;
}

ModuleDataProviderAdapter::~ModuleDataProviderAdapter()
{
    m_host->setModule(nullptr);
    m_host->release();

    if(m_module != nullptr)
    {
        m_destroy(m_module);
        m_module = nullptr;
    }
}

QByteArray ModuleDataProviderAdapter::serializeAndGetData()
{
    return serializeAndGetData({});
}

QByteArray ModuleDataProviderAdapter::serializeAndGetData(const QByteArray &data)
{
    checkRunning();

    return m_module->get(data);
}

QByteArray ModuleDataProviderAdapter::deserializeAndSetData(const QByteArray &data)
{
    checkRunning();

    return m_module->set(data);
}

void ModuleDataProviderAdapter::init()
{
    clean();

    if(m_host->callModule("init",[this] { m_module->init(); }))
    {
        m_state = State::RUNNING;
        LOG_D(QString("Module ").append(m_info->name).append(" is running, available=").append(m_module->isAvailable() ? "true" : "false"));
    }
    else
    {
        m_state = State::FAILED;
        m_host->release();
        LOG_E(QString("Module ").append(m_info->name).append(" initialization failed, module is disabled !"));
    }
}

void ModuleDataProviderAdapter::clean()
{
    if(m_state == State::RUNNING)
    {
        m_host->callModule("clean",[this] { m_module->clean(); });
    }

    m_host->release();

    m_state = State::CREATED;
}

void ModuleDataProviderAdapter::loadSettings()
{
    if(!settingsPolicy().persistent)
    {
        return;
    }

    ModuleSettingsStorage storage(settingsGroup());

    m_host->callModule("load settings",[this,&storage] { m_module->loadSettings(storage); });
}

void ModuleDataProviderAdapter::saveSettings()
{
    if(!settingsPolicy().persistent)
    {
        return;
    }

    ModuleSettingsStorage storage(settingsGroup());

    m_host->callModule("save settings",[this,&storage] { m_module->saveSettings(storage); });
}

quint8 ModuleDataProviderAdapter::checkInfo(const Module::Info *info)
{
    if(info == nullptr || info->name == nullptr || info->version == nullptr)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MODULE,"Module has no valid info !");
    }

    if(info->abiVersion != Module::ABI_VERSION)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MODULE,QString("Module ").append(info->name).append(" has ABI version ").append(QString::number(info->abiVersion)).append(", expected ").append(QString::number(Module::ABI_VERSION)).append(" !").toStdString().c_str());
    }

    /*
     * Dynamic data types are not supported yet (LIST_MODULES)
     */
    if(info->legacyDataType < 0 || info->legacyDataType > std::numeric_limits<quint8>::max())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MODULE,QString("Module ").append(info->name).append(" has no valid legacy data type !").toStdString().c_str());
    }

    return static_cast<quint8>(info->legacyDataType);
}

void ModuleDataProviderAdapter::checkRunning() const
{
    if(m_state != State::RUNNING)
    {
        throw Module::Error(std::string("Module ").append(m_info->name).append(" is not running !"));
    }
}

QString ModuleDataProviderAdapter::settingsGroup() const
{
    return m_info->settingsGroup != nullptr ? QString(m_info->settingsGroup) : QString(m_info->name);
}

}
