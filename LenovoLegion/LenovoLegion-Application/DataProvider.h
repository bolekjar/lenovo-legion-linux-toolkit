// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ProtocolProcessor.h"

#include "QtConcurrent/qtconcurrentrun.h"
#include <QObject>
#include <QFuture>

namespace LenovoLegionGui {

class ProtocolProcessor;

class DataProvider : public QObject
{
    Q_OBJECT

public:

    enum ERROR_CODES : int {
        PARSER_ERROR     = 1,
        SERIALIZE_ERROR  = 2,
        INVOKE_ERROR     = 3
    };


    DEFINE_EXCEPTION(ProtocolProcessorBase);

public:

    DataProvider(ProtocolProcessor & protocolProcessor,QObject *parent);

    template<class Message, class Request = Message>
    Message getDataMessage(quint8 m_dataType,const Request& data = {}) const {
        Message msg;

        const size_t dataSize = data.ByteSizeLong();
        QByteArray requestData;
        requestData.resize(static_cast<int>(dataSize));

        if (!data.SerializeToArray(requestData.data(), requestData.size())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize error");
        }

        std::pair<bool,QByteArray> responseBytes;
        if(!QMetaObject::invokeMethod(&m_protocolProcessor,
                                  &ProtocolProcessor::getDataRequest,
                                  Qt::BlockingQueuedConnection,
                                  qReturnArg(responseBytes),
                                  m_dataType,
                                  requestData)) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::INVOKE_ERROR, "Failed to invoke method ProtocolProcessor::getDataRequest");
        }

        if(responseBytes.first == false) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Response is not valid");
        }

        if (responseBytes.second.isEmpty()) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Received empty response");
        }

        if (!msg.ParseFromArray(responseBytes.second.constData(), responseBytes.second.size())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Parse error");
        }

        return msg;
    }

    template<class Message, class Request = Message>
    QFuture<Message> getDataMessageAsync(quint8 m_dataType, const Request& data = {}) const {
        return QtConcurrent::run(
            &DataProvider::getDataMessage<Message, Request>,
            this,
            m_dataType,
            data
            );
    }


    template<class Message>
    QByteArray setDataMessage(quint8 m_dataType, const Message& message) const {
        const size_t dataSize = message.ByteSizeLong();
        if (dataSize > static_cast<size_t>(std::numeric_limits<int>::max())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Data size exceeds QByteArray limits");
        }

        QByteArray data;
        data.resize(static_cast<int>(dataSize));
        if (!message.SerializeToArray(data.data(), data.size())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize of data message error !");
        }

        std::pair<bool,QByteArray> resultBytes;
        if(!QMetaObject::invokeMethod(
            &m_protocolProcessor,
            &ProtocolProcessor::setDataRequest,
            Qt::BlockingQueuedConnection,
            qReturnArg(resultBytes),
            m_dataType,
            data))
        {
            THROW_EXCEPTION(exception_T, ERROR_CODES::INVOKE_ERROR, "Failed to invoke Method ProtocolProcessor::setDataRequest");
        }

        if(resultBytes.first == false) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Response is not valid");
        }

        return resultBytes.second;
    }

private:

    ProtocolProcessor& m_protocolProcessor;
};

}


