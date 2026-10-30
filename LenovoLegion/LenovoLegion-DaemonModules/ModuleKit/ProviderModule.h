// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Generic daemon module made of one data provider and its internal drivers.
 *
 * Derived module:
 *  - adds its drivers in the constructor (drivers().addDriver),
 *  - creates its provider (setProvider),
 *  - maps driver/provider events to notifications it owns (notificationFor),
 *  - converts its settings (readSettings/writeSettings), the rest is done here.
 */

#include "DataProvider.h"
#include "SysFsDriverManager.h"

#include <IModule.h>
#include <IModuleHost.h>

#include <Notification.pb.h>

#include <memory>
#include <optional>

namespace LenovoLegionDaemon::Kit {

class ProviderModule : public Module::IModule
{
public:

    explicit ProviderModule(Module::IModuleHost& host);
    ~ProviderModule() override;

    /*
     * IModule
     */
    void init()  override;
    void clean() override;
    bool isAvailable() const override;

    QByteArray get(const QByteArray& request) override;
    QByteArray set(const QByteArray& request) override;

    void onKernelEvent(const Module::KernelEvent& event) override;

    void loadSettings(const Module::ISettingsStorage& storage) override;
    void saveSettings(Module::ISettingsStorage& storage) override;

protected:

    Module::IModuleHost& host();
    SysFsDriverManager&  drivers();

    void          setProvider(std::unique_ptr<DataProvider> provider);
    DataProvider& provider();

    /*
     * Notifications owned by the module (only one module may notify an event type)
     */
    virtual std::optional<legion::messages::Notification> notificationFor(const SysFsDriver::SubsystemEvent&)   { return {}; }
    virtual std::optional<legion::messages::Notification> notificationFor(const DataProvider::SubsystemEvent&)  { return {}; }

    /*
     * Kernel module loaded/unloaded (subsystem "module")
     */
    virtual void onKernelModule(const QString& /*name*/,bool /*loaded*/)                                       {}

    /*
     * Settings: build the provider message from the storage (empty = nothing saved),
     * store the provider message read from the hardware
     */
    virtual QByteArray readSettings(const Module::ISettingsStorage&)                                           { return {}; }
    virtual void       writeSettings(Module::ISettingsStorage&,const QByteArray& /*message*/)                  {}

    /*
     * Helpers
     */
    void notify(const legion::messages::Notification& notification);

    /*
     * Driver specific event type (EVENT_TYPE of the Legion kernel module), -1 if not present
     */
    static int eventType(const SysFsDriver::SubsystemEvent& event)
    {
        bool      ok    = false;
        const int value = event.m_DriverSpecificEventType.toInt(&ok);

        return ok ? value : -1;
    }

    static legion::messages::Notification notification(legion::messages::Notification_Action action)
    {
        legion::messages::Notification message;

        message.set_action(action);

        return message;
    }

    /*
     * Keys of a settings array in the same format as QSettings::beginReadArray/beginWriteArray
     * (prefix/size, prefix/1/key, ...), so the settings file stays compatible
     */
    static QString arraySizeKey(const QString& prefix)
    {
        return QString(prefix).append("/size");
    }

    static QString arrayKey(const QString& prefix,int index,const QString& key)
    {
        return QString(prefix).append('/').append(QString::number(index + 1)).append('/').append(key);
    }

    template<class Message>
    static QByteArray serialize(const Message& message)
    {
        return serializeMessage(message);
    }

    template<class Message>
    static Message parse(const QByteArray& bytes)
    {
        return parseMessage<Message>(bytes);
    }

private:

    Module::IModuleHost&           m_host;
    SysFsDriverManager             m_drivers;
    std::unique_ptr<DataProvider>  m_provider;
    bool                           m_initialized;
};

}
