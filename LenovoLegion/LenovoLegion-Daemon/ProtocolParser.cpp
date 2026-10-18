// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolParser.h"

#include <Core/LoggerHolder.h>

#include <QDeadlineTimer>

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

    validateHeader(header);

    /*
     * Read data
     */
    bytes  = readDataWithTimeout(socket,header.m_dataLength);

    /*
     * All looks good
     */
    callback(header,bytes);
}

bool ProtocolParser::tryParseMessage(QLocalSocket &socket, std::function<void (const MessageHeader &,const QByteArray&)> callback)
{
    MessageHeader header;

    if(socket.bytesAvailable() < static_cast<qint64>(sizeof(MessageHeader)))
    {
        return false;
    }

    /*
     * Peek header, it is consumed only together with the whole message
     */
    const QByteArray headerBytes = socket.peek(sizeof(MessageHeader));

    LOG_T(QString("Raw message header = (").append(headerBytes.toHex(':').toStdString()).append(")"));

    memset(&header,0,sizeof(MessageHeader));
    memcpy(&header,headerBytes.constData(),sizeof(MessageHeader));

    validateHeader(header);

    if(socket.bytesAvailable() < static_cast<qint64>(sizeof(MessageHeader)) + header.m_dataLength)
    {
        return false;
    }

    socket.skip(sizeof(MessageHeader));

    callback(header,socket.read(header.m_dataLength));

    return true;
}

void ProtocolParser::validateHeader(const MessageHeader &header)
{
    if(header.m_dataLength < 0 || header.m_dataLength > MAX_MESSAGE_LENGTH)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MESSAGE_LENGTH,QString("Invalid message length ").append(QString::number(header.m_dataLength)).append(" !").toStdString().c_str());
    }
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

QByteArray  ProtocolParser::readDataWithTimeout(QLocalSocket &socket, qsizetype size)
{
    QByteArray        data;
    QDeadlineTimer    deadline(1000);   // 1 second timeout for the whole data

    data.reserve(size);

    while(data.size() < size)
    {
        data.append(socket.read(size - data.size()));

        if(data.size() < size && !socket.waitForReadyRead(static_cast<int>(deadline.remainingTime())))
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::DATA_NOT_READY,"Data not ready - timeout waiting for data!");
        }
    }

    return data;
}

}
