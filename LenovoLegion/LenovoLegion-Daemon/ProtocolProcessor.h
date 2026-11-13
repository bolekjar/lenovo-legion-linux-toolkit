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
#include <QTimer>


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

private:

    virtual void readyReadHandler() override;

    /*
     * Process GET/SET request. Errors of the data provider are sent to the client
     * as ERROR_RESPONSE, the connection stays open.
     */
    void processRequest(const MessageHeader& header,const QByteArray& data);

    void sendMessage(MessageHeader::Type type,quint8 dataType,const QByteArray& payload);

private:

    DataProviderManager*     m_dataProviderManager;

    /*
     * Incomplete message: the client is disconnected when the rest does not come in time, otherwise it would hold
     * the only allowed connection (an idle client without data is fine, the GUI is idle between its requests)
     */
    QTimer                   m_incompleteMessageTimer;

    static constexpr std::chrono::seconds INCOMPLETE_MESSAGE_TIMEOUT{5};
};

}
