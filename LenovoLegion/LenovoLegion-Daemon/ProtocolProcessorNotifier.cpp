// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessorNotifier.h"
#include "ProtocolParser.h"
#include "DataProviderManager.h"

#include <Core/LoggerHolder.h>

#include <QCoreApplication>

namespace LenovoLegionDaemon {

ProtocolProcessorNotifier::ProtocolProcessorNotifier(DataProviderManager* dataProviderManger, QLocalSocket* clientSocket, QObject* parent) :
    ProtocolProcessorBase(clientSocket,parent),
    m_dataProviderManger(dataProviderManger)
{
    /*
     * Notifications are created by daemon modules, they are only forwarded to the client
     */
    connect(m_dataProviderManger,&DataProviderManager::providerNotification,this,&ProtocolProcessorNotifier::providerNotificationHandler);
}

ProtocolProcessorNotifier::~ProtocolProcessorNotifier()
{
    LOG_T("ProtocolProcessorNotifier stopped !");

    ProtocolProcessorBase::stop();
}

void ProtocolProcessorNotifier::stop()
{
    LOG_T("ProtocolProcessorNotifier stopped !");

    ProtocolProcessorBase::stop();
}

void ProtocolProcessorNotifier::start()
{
    LOG_T("ProtocolProcessorNotifier started !");

    ProtocolProcessorBase::start();
}

void ProtocolProcessorNotifier::readyReadHandler()
{
    LOG_T("ProtocolProcessorNotifier readyReadHandler");

    THROW_EXCEPTION(exception_T,ERROR_CODES::UNEXPECTED_MESSAGE,"ProtocolProcessorNotifier: Unexpected message !");
}

void ProtocolProcessorNotifier::providerNotificationHandler(const QByteArray &payload)
{
    if(!isRunning())
    {
        LOG_T("ProtocolProcessorNotifier is not running, ignoring module notification !");
        return;
    }

    /*
     * A client which does not read holds the only notification connection and its unread notifications would grow
     * in the daemon (the write buffer of QLocalSocket is not limited)
     */
    if(m_clientSocket->bytesToWrite() > MAX_UNREAD_BYTES)
    {
        dropClient(QString("Notification client does not read, %1 bytes unread").arg(m_clientSocket->bytesToWrite()));
        return;
    }

    m_clientSocket->write(ProtocolParser::parseMessage(MessageHeader{
                                                           .m_type         = MessageHeader::NOTIFICATION,
                                                           .m_dataType     = m_dataType,
                                                           .m_dataLength   = payload.length()
                                                       },payload));

    LOG_D("ProtocolProcessorNotifier: Module notification sent !");
}

}
