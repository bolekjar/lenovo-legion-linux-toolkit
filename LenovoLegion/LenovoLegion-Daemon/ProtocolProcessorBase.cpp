// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessorBase.h"

#include <Core/LoggerHolder.h>

#include <poll.h>
#include <unistd.h>

namespace LenovoLegionDaemon {

ProtocolProcessorBase::ProtocolProcessorBase(QLocalSocket* clientSocket,QObject* parent) :
    QObject(parent),
    m_clientSocket(clientSocket)
{
    if(m_clientSocket == nullptr)
    {
        THROW_EXCEPTION(exception_T,CLIENT_POINTER_ERROR,"Client socket is nullptr !");

    }
}

ProtocolProcessorBase::~ProtocolProcessorBase()
{
    m_clientSocket->close();
    waitForExit();
    m_clientSocket->deleteLater();

    disconnect(m_clientSocket, &QLocalSocket::readyRead,this,&ProtocolProcessorBase::readyReadHandlerSlot);
    disconnect(m_clientSocket, &QLocalSocket::disconnected,this, &ProtocolProcessorBase::disconnectedHandlerSlot);

    LOG_T("ProtocolProcessorBase destroyed !");
}

void ProtocolProcessorBase::stop()
{
    LOG_T("ProtocolProcessorBase stopped !");

    m_clientSocket->close();
}

void ProtocolProcessorBase::start()
{
    LOG_T("ProtocolProcessorBase started !");

    connect(m_clientSocket, &QLocalSocket::readyRead,this,&ProtocolProcessorBase::readyReadHandlerSlot);
    connect(m_clientSocket, &QLocalSocket::disconnected,this, &ProtocolProcessorBase::disconnectedHandler);
}

void ProtocolProcessorBase::waitForExit()
{
    if (!m_clientSocket || !m_clientSocket->isOpen()) {
        return;
    }

    // Use poll to wait for socket to close without busy-waiting
    pollfd pfd;
    pfd.fd = m_clientSocket->socketDescriptor();
    pfd.events = POLLIN | POLLHUP | POLLERR;
    
    while (m_clientSocket->isOpen()) {
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
                m_clientSocket->waitForReadyRead(0);
            }
        }
        
        // Check if socket closed during event processing
        if (!m_clientSocket->isOpen()) {
            break;
        }
    }
}

bool ProtocolProcessorBase::isRunning() const
{
    return  m_clientSocket->isOpen();
}

void ProtocolProcessorBase::disconnectedHandler()
{}

void ProtocolProcessorBase::readyReadHandlerSlot()
{
    readyReadHandler();
}

void ProtocolProcessorBase::disconnectedHandlerSlot()
{
    LOG_D("Client disconnected !");
    m_clientSocket->close();

    disconnectedHandler();
}



void ProtocolProcessorBase::refuseConnection(QLocalSocket *clientSocket)
{
    LOG_D("Refusing client connection !");

    if(clientSocket == nullptr)
    {
        THROW_EXCEPTION(exception_T,CLIENT_POINTER_ERROR,"Client socket is nullptr !");

    }

    clientSocket->close();
    clientSocket->deleteLater();
}

}
