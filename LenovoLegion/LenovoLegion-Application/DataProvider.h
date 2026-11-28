// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ProtocolProcessor.h"
#include "Core/LoggerHolder.h"

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
        INVOKE_ERROR     = 3,
        DAEMON_ERROR     = 4,
        NOT_AVAILABLE    = 5      // the data are not available on this machine (not an error), the reason is the message
    };


    DEFINE_EXCEPTION(ProtocolProcessorBase);

public:

    DataProvider(ProtocolProcessor & protocolProcessor,QObject *parent);

    /*
     * Serialized request and response of a data type, used by the generic GUI (Ui/DataSource) which does not know the message type
     */
    QByteArray getData(quint8 dataType,const QByteArray& request = {}) const;
    QByteArray setData(quint8 dataType,const QByteArray& data) const;

    template<class Message, class Request = Message>
    Message getDataMessage(quint8 m_dataType,const Request& data = {}) const {
        Message msg;

        const size_t dataSize = data.ByteSizeLong();
        QByteArray requestData;
        requestData.resize(static_cast<int>(dataSize));

        if (!data.SerializeToArray(requestData.data(), requestData.size())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::SERIALIZE_ERROR, "Serialize error");
        }

        RequestResult responseBytes;
        if(!QMetaObject::invokeMethod(&m_protocolProcessor,
                                  &ProtocolProcessor::getDataRequest,
                                  Qt::BlockingQueuedConnection,
                                  qReturnArg(responseBytes),
                                  m_dataType,
                                  requestData)) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::INVOKE_ERROR, "Failed to invoke method ProtocolProcessor::getDataRequest");
        }

        if(responseBytes.status != RequestResult::OK) {
            throwRequestError(responseBytes);
        }

        if (!msg.ParseFromArray(responseBytes.data.constData(), responseBytes.data.size())) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Parse error");
        }

        return msg;
    }

    template<class Message, class Request = Message>
    QFuture<Message> getDataMessageAsync(quint8 m_dataType, Request data = {}) const {
        return QtConcurrent::run([this, m_dataType, data = std::move(data)]() {
            try {
                return this->getDataMessage<Message, Request>(m_dataType, std::move(data));
            } catch (const exception_T& e) {
                if(e.errcodeInfo().value() == ERROR_CODES::NOT_AVAILABLE) {
                    LOG_D(QString("DataProvider: not available: ").append(e.what()));
                } else {
                    LOG_E(QString("DataProvider exception: ").append(e.what()));
                }
            } catch (const std::exception& e) {
                LOG_E(QString("Standard exception: ").append(e.what()));
            } catch (...) {
                LOG_E("Unknown exception occurred in getDataMessageAsync");
            }
            return Message(); // Return a default-constructed Message in case of an exception
        });
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

        RequestResult resultBytes;
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

        if(resultBytes.status != RequestResult::OK) {
            throwRequestError(resultBytes);
        }

        return resultBytes.data;
    }

private:

    /*
     * Failed request: the reason of not available data, the description from the daemon (ERROR_RESPONSE) or empty
     */
    [[noreturn]] static void throwRequestError(const RequestResult& result) {
        if(result.status == RequestResult::NOT_AVAILABLE) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::NOT_AVAILABLE, QString::fromUtf8(result.data).toStdString().c_str());
        }

        if(!result.data.isEmpty()) {
            THROW_EXCEPTION(exception_T, ERROR_CODES::DAEMON_ERROR, QString::fromUtf8(result.data).toStdString().c_str());
        }

        THROW_EXCEPTION(exception_T, ERROR_CODES::PARSER_ERROR, "Response is not valid");
    }

    ProtocolProcessor& m_protocolProcessor;
};

}


