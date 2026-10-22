// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessor.h"
#include "ProtocolParser.h"
#include "Message.h"
#include "DataProviderManager.h"

#include <Core/LoggerHolder.h>

#include <QCoreApplication>


namespace LenovoLegionDaemon {

ProtocolProcessor::ProtocolProcessor(DataProviderManager* dataProviderManager,QLocalSocket* clientSocket,QObject* parent) :
    ProtocolProcessorBase(clientSocket,parent),
    m_dataProviderManager(dataProviderManager)
{}

ProtocolProcessor::~ProtocolProcessor()
{
}

void ProtocolProcessor::stop()
{
    LOG_T("ProtocolProcessor stopped !");

    ProtocolProcessorBase::stop();
}

void ProtocolProcessor::start()
{
    LOG_T("ProtocolProcessor started !");

    ProtocolProcessorBase::start();
}

void ProtocolProcessor::disconnectedHandler()
{
    LOG_T("ProtocolProcessor client disconnected !");
    ProtocolProcessorBase::stop();
    ProtocolProcessorBase::waitForExit();

    emit clientDisconnected();
}

void ProtocolProcessor::readyReadHandler()
{

    if(!isRunning())
    {
        LOG_W("ProtocolProcessor is stopped, message will not be processed !");
        return;
    }
    
    if(!m_dataProviderManager)
    {
        LOG_W("ProtocolProcessor: DataProviderManager is null, message will not be processed !");
        return;
    }

    /*
     * Process all complete messages, incomplete one waits for the next readyRead (no blocking of the event loop)
     */
    while(isRunning() && ProtocolParser::tryParseMessage(*m_clientSocket,[this](const MessageHeader& header,const QByteArray& data ){

        LOG_T(QString("Message header was readed: header.m_type= ").append(QString::number(header.m_type)).append(", header.m_dataType=").append(QString::number(header.m_dataType)).append(", header.m_dataLength=").append(QString::number(header.m_dataLength)));

        switch (header.m_type) {
        case MessageHeader::GET_DATA_REQUEST:
        case MessageHeader::SET_DATA_REQUEST:
            processRequest(header,data);
            break;
        case MessageHeader::GET_DATA_RESPONSE:
        case MessageHeader::SET_DATA_RESPONSE:
        case MessageHeader::NOTIFICATION:
        case MessageHeader::ERROR_RESPONSE:
            break;
        default:
                LOG_W(QString("Unkonow message type(").append(QString::number(header.m_type)).append(")"));
            break;
        }

        LOG_T(QString("Message received: done !"));
    }));
}

void ProtocolProcessor::processRequest(const MessageHeader &header, const QByteArray &data)
{
    QString error;

    /*
     * Exception must not leave this method, it would go through Qt signal/slot code
     * and Application::notify would disconnect the client
     */
    try {
        DataProvider& dataProvider = m_dataProviderManager->getDataProvider(header.m_dataType);

        if(header.m_type == MessageHeader::GET_DATA_REQUEST)
        {
            sendMessage(MessageHeader::GET_DATA_RESPONSE,header.m_dataType,data.size() > 0 ? dataProvider.serializeAndGetData(data) : dataProvider.serializeAndGetData());
        }
        else
        {
            sendMessage(MessageHeader::SET_DATA_RESPONSE,header.m_dataType,dataProvider.deserializeAndSetData(data));
        }

        return;
    }
    catch(const bj::framework::exception::Exception& ex)
    {
        LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
        error = QString::fromStdString(ex.descriptionInfo().value());

        if(error.isEmpty())
        {
            error = "Unknown error !";
        }
    }
    catch(const std::exception& ex)
    {
        LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__PRETTY_FUNCTION__,__LINE__,1,ex).c_str());
        error = QString::fromUtf8(ex.what());
    }
    catch(...)
    {
        LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__PRETTY_FUNCTION__,__LINE__,1,"Unknown error !").c_str());
        error = "Unknown error !";
    }

    LOG_E(QString("Request type=").append(QString::number(header.m_type)).append(", dataType=").append(QString::number(header.m_dataType)).append(" failed, sending error response: ").append(error));

    sendMessage(MessageHeader::ERROR_RESPONSE,header.m_dataType,error.toUtf8());
}

void ProtocolProcessor::sendMessage(MessageHeader::Type type, quint8 dataType, const QByteArray &payload)
{
    m_clientSocket->write(
        ProtocolParser::parseMessage(
            MessageHeader {
                .m_type         = type,
                .m_dataType     = dataType,
                .m_dataLength   = payload.length()
            },
            payload
            )
        );
}

}
