// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "Message.h"

#include <Core/ExceptionBuilder.h>


#include <QByteArray>
#include <QLocalSocket>

namespace LenovoLegionDaemon {

struct ProtocolParser
{

    DEFINE_EXCEPTION(Protocol)

    enum ERROR_CODES : int {
        DATA_NOT_READY                  = 1,
        INVALID_MESSAGE_LENGTH          = 2
    };

    /*
     * Maximal payload length of one message
     */
    static constexpr qsizetype MAX_MESSAGE_LENGTH = 1024 * 1024;


    /*
     * Blocking read of one message, waits for the whole message (with timeout)
     */
    static void           parseMessage(QLocalSocket& socket,std::function<void(const MessageHeader& message,const QByteArray& data)> callback);

    /*
     * Non-blocking read of one message, returns false if the whole message is not available yet
     */
    static bool           tryParseMessage(QLocalSocket& socket,std::function<void(const MessageHeader& message,const QByteArray& data)> callback);

    static QByteArray     parseMessage(const MessageHeader& message,const QByteArray& data);

private:

    static void           validateHeader(const MessageHeader& header);
    static QByteArray     readDataWithTimeout(QLocalSocket& socket,qsizetype size);
};


}
