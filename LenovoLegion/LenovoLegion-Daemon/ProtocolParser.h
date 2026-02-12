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
        DATA_NOT_READY                  = 1
    };


    static void           parseMessage(QLocalSocket& socket,std::function<void(const MessageHeader& message,const QByteArray& data)> callback);
    static QByteArray     parseMessage(const MessageHeader& message,const QByteArray& data);

private:

    static QByteArray     readDataWithTimeout(QLocalSocket& socket,int size);
};


}
