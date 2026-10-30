// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * Serialization of protobuf messages used by providers and modules
 */

#include "KitException.h"

#include <QByteArray>

namespace LenovoLegionDaemon::Kit {

enum MESSAGE_ERROR_CODES : int {
    MESSAGE_PARSE_ERROR     = -1,
    MESSAGE_SERIALIZE_ERROR = -2
};

template<class Message>
QByteArray serializeMessage(const Message& message)
{
    QByteArray bytes;

    bytes.resize(static_cast<qsizetype>(message.ByteSizeLong()));

    if(!message.SerializeToArray(bytes.data(),static_cast<int>(bytes.size())))
    {
        THROW_EXCEPTION(ProviderException,MESSAGE_SERIALIZE_ERROR,"Serialize of data message error !");
    }

    return bytes;
}

template<class Message>
Message parseMessage(const QByteArray& bytes)
{
    Message message;

    if(!message.ParseFromArray(bytes.constData(),static_cast<int>(bytes.size())))
    {
        THROW_EXCEPTION(ProviderException,MESSAGE_PARSE_ERROR,"Parse of data message error !");
    }

    return message;
}

}
