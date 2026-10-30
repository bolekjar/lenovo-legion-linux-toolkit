// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProviderModule.h"

namespace LenovoLegionDaemon::Kit {

ProviderModule::ProviderModule(Module::IModuleHost &host) :
    m_host(host),
    m_drivers(host),
    m_initialized(false)
{
    setLogger(&host.logger());

    m_drivers.setEventSink([this](const SysFsDriver::SubsystemEvent& event) {

        if(m_provider != nullptr)
        {
            m_provider->kernelEventHandler(event);
        }

        if(const auto notification = notificationFor(event))
        {
            notify(*notification);
        }
    });
}

ProviderModule::~ProviderModule()
{
    m_provider.reset();
    setLogger(nullptr);
}

void ProviderModule::init()
{
    if(m_provider == nullptr)
    {
        throw Module::Error("Module has no data provider !");
    }

    m_drivers.initDrivers();
    m_provider->init();
    m_initialized = true;
}

void ProviderModule::clean()
{
    if(m_provider != nullptr && m_initialized)
    {
        m_provider->clean();
    }

    m_drivers.cleanDrivers();
    m_initialized = false;
}

bool ProviderModule::isAvailable() const
{
    return m_initialized;
}

QByteArray ProviderModule::get(const QByteArray &request)
{
    return request.isEmpty() ? provider().serializeAndGetData() : provider().serializeAndGetData(request);
}

QByteArray ProviderModule::set(const QByteArray &request)
{
    return provider().deserializeAndSetData(request);
}

void ProviderModule::onKernelEvent(const Module::KernelEvent &event)
{
    m_drivers.handleKernelEvent(event);

    if(event.m_subSystem == SysFsDriverManager::MODULE_SUBSYSTEM && (event.m_action == "add" || event.m_action == "remove"))
    {
        onKernelModule(event.m_sysName,event.m_action == "add");
    }
}

void ProviderModule::loadSettings(const Module::ISettingsStorage &storage)
{
    const QByteArray message = readSettings(storage);

    if(message.isEmpty())
    {
        LOG_D("No saved settings, skipping");
        return;
    }

    provider().deserializeAndSetData(message);
}

void ProviderModule::saveSettings(Module::ISettingsStorage &storage)
{
    /*
     * Only the settable part of the current state, read-only values are never saved
     */
    writeSettings(storage,provider().settableData(provider().serializeAndGetData(),DataProvider::Origin::STATE));
}

Module::IModuleHost &ProviderModule::host()
{
    return m_host;
}

SysFsDriverManager &ProviderModule::drivers()
{
    return m_drivers;
}

void ProviderModule::setProvider(std::unique_ptr<DataProvider> provider)
{
    m_provider = std::move(provider);

    m_provider->attach(&m_host,[this](const DataProvider::SubsystemEvent& event) {
        if(const auto notification = notificationFor(event))
        {
            notify(*notification);
        }
    });
}

DataProvider &ProviderModule::provider()
{
    if(m_provider == nullptr)
    {
        throw Module::Error("Module has no data provider !");
    }

    return *m_provider;
}

void ProviderModule::notify(const legion::messages::Notification &notification)
{
    m_host.notify(serialize(notification));
}

}
