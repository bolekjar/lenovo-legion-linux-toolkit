// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>

#include <QString>
#include <QByteArray>
#include <functional>





namespace bj { namespace connectX {

struct ConnectXProtocol
{
    DEFINE_EXCEPTION(ConnectXServerProtocol)

    enum ERROR_CODES : int {
        PROTOCOL_SIZE_ERROR = 0,
        PROTOCOL_ERROR_1    = 1,
        PROTOCOL_ERROR_UNKNOWN_COMMAND = 2,
        PROTOCOL_ERROR_2               = 3,
        MORE_DATA                      = 4,
        PROTOCOL_ERROR_CALLBACK        = 5
    };


    enum class COMMAND {
        HELLO           = 0,
        CONFIGURATION   = 1,
        MOVE            = 2,
        WAITING_ROOM    = 3,
        PROTOCOL_ERROR  = 4,
        WAIT_FOR_OTHERS = 5,
        START_GAME      = 6,
        END_GAME        = 7
    };

    struct Message {
        COMMAND m_commnad;
        QString m_sender;
        QString m_data;
    };


    static const int MAX_LENGTH_OF_RAW_MESSAGE;
    static const QString  CONNECTX_SERVER_NAME;


    static void           parseMessage(QByteArray& rawMessage,std::function<void(const Message& message)> callback);
    static QByteArray     parseMessage(const Message& message);


private:
};

}}
