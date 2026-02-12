// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ConnectXProtocol.h"

#include <QStringList>

namespace bj { namespace connectX {

const int ConnectXProtocol::MAX_LENGTH_OF_RAW_MESSAGE = 1024*1024;
const QString ConnectXProtocol::CONNECTX_SERVER_NAME  = "ConnectXServer";


void ConnectXProtocol::parseMessage(QByteArray &rawMessage, std::function<void (const ConnectXProtocol::Message &)> callback)
{

    if(rawMessage.size() > MAX_LENGTH_OF_RAW_MESSAGE)
        THROW_EXCEPTION(exception_T,ERROR_CODES::PROTOCOL_SIZE_ERROR,"Protocol max size error!")

    if(rawMessage.size() == 0)
       THROW_EXCEPTION(exception_T,ERROR_CODES::PROTOCOL_SIZE_ERROR,"Protocol min size error!")

    QStringList resultStrings = QString(rawMessage).split("\r\n");


    if(resultStrings.last().isEmpty())
    {
        rawMessage.clear();
        resultStrings.removeLast();
    }
    else
    {
        rawMessage = resultStrings.last().toLocal8Bit();
        resultStrings.removeLast();
    }

    for (auto& cur : resultStrings)
    {
        Message message;

        QString command  = cur.section(":",0,0);
        message.m_sender = cur.section(":",1).section("+",0,0);
        message.m_data   = cur.section(":",1).section("+",1);

        if(command == "HELLO")
        {
            message.m_commnad = COMMAND::HELLO;
        }
        else if(command == "CONFIGURATION")
        {
            message.m_commnad = COMMAND::CONFIGURATION;
        }
        else if(command == "MOVE")
        {
            message.m_commnad = COMMAND::MOVE;
        } else if(command == "WAITING_ROOM")
        {
            message.m_commnad = COMMAND::WAITING_ROOM;
        }
        else if(command == "PROTOCOL_ERROR")
        {
            message.m_commnad = COMMAND::PROTOCOL_ERROR;
        }
        else if(command == "WAIT_FOR_OTHERS")
        {
            message.m_commnad = COMMAND::WAIT_FOR_OTHERS;
        }
        else if(command == "START_GAME")
        {
            message.m_commnad = COMMAND::START_GAME;
        }
        else if(command == "END_GAME")
        {
            message.m_commnad = COMMAND::END_GAME;
        }
        else
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::PROTOCOL_ERROR_UNKNOWN_COMMAND,QString("Protocol error = unknown command (").append(command).append(")!").toStdString().c_str())
        }

        if(message.m_sender == "")
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::PROTOCOL_ERROR_UNKNOWN_COMMAND,QString("Protocol error = sender is not specified !").toStdString().c_str())
        }

        callback(message);
    }

    if(rawMessage.size() > 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::MORE_DATA,"More data !")
    }

}

QByteArray ConnectXProtocol::parseMessage(const ConnectXProtocol::Message &message)
{
    QString rawMessage;

    switch (message.m_commnad)
    {
    case COMMAND::MOVE:
    {
       rawMessage.append("MOVE");
       break;
    }
    case COMMAND::HELLO:
    {
        rawMessage.append("HELLO");
        break;
    }
    case COMMAND::CONFIGURATION:
    {
        rawMessage.append("CONFIGURATION");
        break;
    }
    case COMMAND::WAITING_ROOM:
    {
        rawMessage.append("WAITING_ROOM");
        break;
    }
    case COMMAND::PROTOCOL_ERROR:
    {
        rawMessage.append("PROTOCOL_ERROR");
        break;
    }
    case COMMAND::WAIT_FOR_OTHERS:
    {
        rawMessage.append("WAIT_FOR_OTHERS");
        break;
    }
    case COMMAND::START_GAME:
    {
        rawMessage.append("START_GAME");
        break;
    }
    case COMMAND::END_GAME:
    {
        rawMessage.append("END_GAME");
        break;
    }
    }

    rawMessage.append(":");
    rawMessage.append(message.m_sender);
    rawMessage.append("+");
    rawMessage.append(message.m_data);
    rawMessage.append("\r\n");

    return rawMessage.toLocal8Bit();
}

}}
