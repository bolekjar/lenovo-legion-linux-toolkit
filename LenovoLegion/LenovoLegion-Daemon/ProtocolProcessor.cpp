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

    ProtocolParser::parseMessage(*m_clientSocket,[this](const MessageHeader& header,const QByteArray& data ){

        LOG_T(QString("Message header was readed: header.m_type= ").append(QString::number(header.m_type)).append(", header.m_dataType=").append(QString::number(header.m_dataType)).append(", header.m_dataLength=").append(QString::number(header.m_dataLength)));

        switch (header.m_type) {
        case MessageHeader::GET_DATA_REQUEST: {
            if(data.size() > 0)
            {
                QByteArray reponse = m_dataProviderManager->getDataProvider(header.m_dataType).serializeAndGetData(data);
                m_clientSocket->write(
                    ProtocolParser::parseMessage(
                        MessageHeader {
                            .m_type         =  MessageHeader::GET_DATA_RESPONSE,
                            .m_dataType     =  header.m_dataType,
                            .m_dataLength   = reponse.length()
                        },
                        reponse
                        )
                    );
            }
            else
            {
                QByteArray reponse = m_dataProviderManager->getDataProvider(header.m_dataType).serializeAndGetData();
                m_clientSocket->write(
                    ProtocolParser::parseMessage(
                        MessageHeader {
                            .m_type         =  MessageHeader::GET_DATA_RESPONSE,
                            .m_dataType     =  header.m_dataType,
                            .m_dataLength   = reponse.length()
                        },
                        reponse
                        )
                    );
            }
        }
            break;
        case MessageHeader::SET_DATA_REQUEST: {
            QByteArray reponse = m_dataProviderManager->getDataProvider(header.m_dataType).deserializeAndSetData(data);
            m_clientSocket->write(
                ProtocolParser::parseMessage(
                    MessageHeader {
                        .m_type         =  MessageHeader::SET_DATA_RESPONSE,
                        .m_dataType     =  header.m_dataType,
                        .m_dataLength   = reponse.length()
                    },
                    reponse
                    )
                );
        }
            break;
        case MessageHeader::GET_DATA_RESPONSE:

            break;

        case MessageHeader::SET_DATA_RESPONSE:

            break;
        case MessageHeader::NOTIFICATION:

            break;
        default:
                LOG_W(QString("Unkonow message type(").append(QString::number(header.m_type)).append(")"));
            break;
        }

        LOG_T(QString("Message received: done !"));
    });
}

}
