// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolParser.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


void ProtocolParser::parseMessage(QLocalSocket &socket, std::function<void (const MessageHeader &,const QByteArray&)> callback)
{
    MessageHeader header;
    QByteArray bytes = readDataWithTimeout(socket,sizeof(MessageHeader));

    LOG_T(QString("Raw message header = (").append(bytes.toHex(':').toStdString()).append(")"));


    /*
     * Deserialize Header
     */
    memset(&header,0,sizeof(MessageHeader));
    memcpy(&header,bytes.data(),sizeof(MessageHeader));

    /*
     * Read data
     */
    bytes  = readDataWithTimeout(socket,header.m_dataLength);

    /*
     * All looks good
     */
    callback(header,bytes);
}

QByteArray ProtocolParser::parseMessage(const MessageHeader &message, const QByteArray &payload)
{
    QByteArray bytes;


    /*
     * Serialize Header
     */
    bytes.append(reinterpret_cast<const char*>(&message),sizeof(MessageHeader));



    /*
     * Add payload
     */
    bytes.append(payload);

    return bytes;
}

QByteArray  ProtocolParser::readDataWithTimeout(QLocalSocket &socket, int size)
{
    QByteArray data;

    // Try to read immediately
    data.append(socket.read(size));

    if(data.size() == size)
    {
        return data;
    }

    // Wait for data to be available (non-blocking with timeout)
    // This returns control to event loop while waiting
    if(!socket.waitForReadyRead(1000))  // 1 second timeout
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DATA_NOT_READY,"Data not ready - timeout waiting for data!");
    }

    // Read remaining data
    data.append(socket.read(size - data.size()));

    if(data.size() != size)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DATA_NOT_READY,"Data not ready - incomplete read!");
    }

    return data;
}

}
