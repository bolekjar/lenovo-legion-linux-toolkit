// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ProtocolProcessorBase.h"
#include "Message.h"

#include <Core/ExceptionBuilder.h>

#include <QObject>
#include <QLocalSocket>
#include <QFileSystemWatcher>


namespace LenovoLegionDaemon {

class DataProviderManager;
class SysFsDriverManager;
class ProtocolProcessor : public ProtocolProcessorBase
{
    Q_OBJECT

public:

    ProtocolProcessor(DataProviderManager* dataProviderManager, QLocalSocket* clientSocket,QObject* parent = nullptr);
    ~ProtocolProcessor();

    virtual void stop() override;
    virtual void start() override;

signals:

    void clientDisconnected();

private:

    virtual void disconnectedHandler() override;
    virtual void readyReadHandler() override;

    /*
     * Process GET/SET request. Errors of the data provider are sent to the client
     * as ERROR_RESPONSE, the connection stays open.
     */
    void processRequest(const MessageHeader& header,const QByteArray& data);

    void sendMessage(MessageHeader::Type type,quint8 dataType,const QByteArray& payload);

private:

    DataProviderManager*     m_dataProviderManager;
};

}
