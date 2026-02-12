// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessorNotifier.h"

#include <ProtocolParser.h>

#include <Core/LoggerHolder.h>

#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

#include <../LenovoLegion-Daemon/Application.h>

namespace LenovoLegionGui {

ProtocolProcessorNotifier::ProtocolProcessorNotifier(QObject *parent)
    : ProtocolProcessorBase(LenovoLegionDaemon::Application::SOCKET_NAME_NOTIFICATION,parent)
{
     m_timerId = startTimer(10);
}

ProtocolProcessorNotifier::~ProtocolProcessorNotifier()
{}


void ProtocolProcessorNotifier::timerEvent(QTimerEvent *event)
{

    if(m_timerId == event->timerId())
    {
        if(m_socket->bytesAvailable() > 0)
        {
            QByteArray data;
            auto msg = receiveMessageDataReady(data);

            if(msg.m_type == LenovoLegionDaemon::MessageHeader::Type::NOTIFICATION)
            {
                legion::messages::Notification msg;

                msg.ParseFromArray(data,data.size());

                emit daemonNotification(msg);

                return;
            }

            THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MESSAGE,"Invalid message");
        }
    }

    ProtocolProcessorBase::timerEvent(event);
}

}
