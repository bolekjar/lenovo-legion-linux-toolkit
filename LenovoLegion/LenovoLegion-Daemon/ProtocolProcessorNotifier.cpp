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

void ProtocolProcessorNotifier::disconnectedHandler()
{
    LOG_D("ProtocolProcessorNotifier client disconnected !");
    ProtocolProcessorBase::stop();
    ProtocolProcessorBase::waitForExit();

    emit clientDisconnected();
}

void ProtocolProcessorNotifier::providerNotificationHandler(const QByteArray &payload)
{
    if(!isRunning())
    {
        LOG_T("ProtocolProcessorNotifier is not running, ignoring module notification !");
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
