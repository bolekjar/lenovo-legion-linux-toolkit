// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessorBase.h"

#include <ProtocolParser.h>

#include <Core/LoggerHolder.h>

#include <../LenovoLegion-Daemon/Application.h>

#include <poll.h>
#include <unistd.h>

namespace LenovoLegionGui {

ProtocolProcessorBase::ProtocolProcessorBase(const QString& socketName,QObject *parent)
    : QObject(parent)
    , SOCKET_NAME{socketName}
    , m_socket{new QLocalSocket{this}}
{
    connect(m_socket, &QLocalSocket::connected, this, &ProtocolProcessorBase::onConnected);
    connect(m_socket, &QLocalSocket::disconnected, this, &ProtocolProcessorBase::onDisconnected);

    m_timerId = startTimer(1000);
}

ProtocolProcessorBase::~ProtocolProcessorBase()
{
    m_socket->close();
    waitForExit();
}

void ProtocolProcessorBase::timerEvent(QTimerEvent *timerId)
{
    if(timerId->timerId() == m_timerId)
    {
        if(!m_socket->isOpen())
        {
            LOG_T(QString("Connecting to daemon socket ( ").append(SOCKET_NAME).append(" )"));

            m_socket->connectToServer(SOCKET_NAME);
        }
    }
}

void ProtocolProcessorBase::sendMessage(const LenovoLegionDaemon::MessageHeader &message,const QByteArray& data)
{
    if(!isConnected())
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::NOT_CONNECTED,"Socket is not connected !");
    }

    m_socket->write(LenovoLegionDaemon::ProtocolParser::parseMessage(message,data));
}

LenovoLegionDaemon::MessageHeader ProtocolProcessorBase::receiveMessage(QByteArray &data,int timeout)
{
    if(!isConnected())
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::NOT_CONNECTED,"Socket is not connected !");
    }

    if(m_socket->waitForReadyRead(timeout))
    {
        LenovoLegionDaemon::MessageHeader message;
        LenovoLegionDaemon::ProtocolParser::parseMessage(*m_socket,[&message,&data](const LenovoLegionDaemon::MessageHeader &msg,const QByteArray& daemonData){
            message = msg;
            data = daemonData;
        });

        return message;
    }

    LOG_E("Timeout while waiting for message, closing socket !");

    m_socket->close();
    waitForExit();

    THROW_EXCEPTION(exception_T, ERROR_CODES::TIMEOUT_ERROR,"Timeout while waiting for message");
}

LenovoLegionDaemon::MessageHeader ProtocolProcessorBase::receiveMessageDataReady(QByteArray& data)
{
    LenovoLegionDaemon::MessageHeader message;
    LenovoLegionDaemon::ProtocolParser::parseMessage(*m_socket,[&message,&data](const LenovoLegionDaemon::MessageHeader &msg,const QByteArray& daemonData){
        message = msg;
        data = daemonData;
    });

    return message;
}

void ProtocolProcessorBase::onConnected()
{
    LOG_T(QString("Connected to daemon socket ( ").append(SOCKET_NAME).append(" )"));
    emit connected();
}

void ProtocolProcessorBase::onDisconnected()
{
    m_socket->close();

    LOG_T(QString("Disconnected from daemon socket ( ").append(SOCKET_NAME).append(" )"));
    emit disconnected();
}

void ProtocolProcessorBase::waitForExit()
{
    if (!m_socket || !m_socket->isOpen()) {
        return;
    }

    // Use poll to wait for socket to close without busy-waiting
    pollfd pfd;
    pfd.fd = m_socket->socketDescriptor();
    pfd.events = POLLIN | POLLHUP | POLLERR;
    
    while (m_socket->isOpen()) {
        pfd.revents = 0;
        
        // Poll with timeout to periodically check if socket closed
        int ret = poll(&pfd, 1, 100);  // 100ms timeout
        
        if (ret < 0) {
            // Poll error
            break;
        }
        
        if (ret > 0) {
            // Socket has events - check if it's disconnection
            if (pfd.revents & (POLLHUP | POLLERR | POLLNVAL)) {
                // Socket disconnected or error
                break;
            }
            
            if (pfd.revents & POLLIN) {
                // Data available - let Qt event loop handle it
                m_socket->waitForReadyRead(0);
            }
        }
        
        // Check if socket closed during event processing
        if (!m_socket->isOpen()) {
            break;
        }
    }
}

bool ProtocolProcessorBase::isConnected()
{
    return m_socket->isOpen();
}

void ProtocolProcessorBase::reconnect()
{
    LOG_D(QString("Reconnecting to daemon socket ( ").append(SOCKET_NAME).append(" )"));
    
    if (m_socket->isOpen()) {
        m_socket->close();
        waitForExit();
    }
    
    // Timer will automatically reconnect on next tick
}

}
