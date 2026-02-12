// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ProtocolProcessor.h"


#include <QObject>

namespace LenovoLegionGui {

class ProtocolProcessor;

class DataProvider : public QObject
{
    Q_OBJECT

public:

    enum ERROR_CODES : int {
        PARSER_ERROR     = 1,
        SERIALIZE_ERROR  = 2
    };


    DEFINE_EXCEPTION(ProtocolProcessorBase);

public:

    DataProvider(ProtocolProcessor * protocolProcessor,QObject *parent);


    template<class Message, class Request = Message>
    Message getDataMessage(quint8 m_dataType,const Request& data = {}) const {

        QByteArray                    byte = m_protocolProcessor->getDataRequest(m_dataType,[&data]{

            QByteArray    requestData;

            requestData.resize(data.ByteSizeLong());
            if(!data.SerializeToArray(requestData.data(),requestData.size()))
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
            }

            return requestData;
        }());


        Message                       msg;

        if(!msg.ParsePartialFromArray(byte,byte.size()))
        {
             THROW_EXCEPTION(exception_T,ERROR_CODES::PARSER_ERROR,"Parse of data message error !");
        }

        return msg;
    }

    template<class Message>
    QByteArray setDataMessage(quint8 m_dataType,const Message& message) const {
        QByteArray                    data;

        data.resize(message.ByteSizeLong());
        if(!message.SerializeToArray(data.data(),data.size()))
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::SERIALIZE_ERROR,"Serialize of data message error !");
        }

        return m_protocolProcessor->setDataRequest(m_dataType,data);
    }

private:

    ProtocolProcessor* m_protocolProcessor;
};

}


