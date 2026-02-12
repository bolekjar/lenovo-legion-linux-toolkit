// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"
#include "SysFsDriverManager.h"
#include "DataProviderManager.h"

#include "ProtocolProcessorBase.h"



namespace LenovoLegionDaemon {


class SysFsDriverManager;
class ProtocolProcessorNotifier : public ProtocolProcessorBase
{
    Q_OBJECT

public:

    enum ERROR_CODES : int {
        UNEXPECTED_MESSAGE = -1,
        WATCHED_PATH_ERROR = -2,
        SERIALIZE_ERROR    = -3
    };


public:

    ProtocolProcessorNotifier(SysFsDriverManager* sysfsDriverManager,DataProviderManager* dataProviderManger,QLocalSocket* clientSocket,QObject* parent);
    ~ProtocolProcessorNotifier();

    virtual void stop()  override;
    virtual void start() override;

private:

    virtual void readyReadHandler() override;
    virtual void disconnectedHandler() override;

signals:

    void clientDisconnected();

public slots:

    void kernelEventHandler(const LenovoLegionDaemon::SysFsDriver::SubsystemEvent& event);
    void moduleSubsystemHandler(const LenovoLegionDaemon::SysFsDriverManager::ModuleSubsystemEvent& event);
public:

    static constexpr quint8  m_dataType = 0;

private:

    SysFsDriverManager*     m_sysfsDriverManager;
    DataProviderManager*    m_dataProviderManger;
};

}

