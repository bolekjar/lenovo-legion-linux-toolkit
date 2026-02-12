// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ProtocolProcessor.h"

#include <ProtocolParser.h>

#include <Core/LoggerHolder.h>

#include <../LenovoLegion-Daemon/Application.h>
#include <../LenovoLegion-Daemon/SysFsDataProviderHWMon.h>


#include <../LenovoLegion-PrepareBuild/HWMonitoring.pb.h>

namespace LenovoLegionGui {

ProtocolProcessor::ProtocolProcessor(QObject *parent)
    : ProtocolProcessorBase(LenovoLegionDaemon::Application::SOCKET_NAME,parent)
{}

ProtocolProcessor::~ProtocolProcessor()
{}

QByteArray ProtocolProcessor::getDataRequest(quint8 dataType, const QByteArray& data)
{
    QByteArray response;

    sendMessage(LenovoLegionDaemon::MessageHeader {
                    .m_type     = LenovoLegionDaemon::MessageHeader::GET_DATA_REQUEST,
                    .m_dataType = dataType,
                    .m_dataLength = data.length()
                },data);

    auto msg = receiveMessage(response);

    if(msg.m_type != LenovoLegionDaemon::MessageHeader::GET_DATA_RESPONSE)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MESSAGE,"Invalid get data request response message");
    }

    return response;
}

QByteArray ProtocolProcessor::setDataRequest(quint8 dataType, const QByteArray &data)
{
    QByteArray dataResponse;

    sendMessage(LenovoLegionDaemon::MessageHeader {
                    .m_type     = LenovoLegionDaemon::MessageHeader::SET_DATA_REQUEST,
                    .m_dataType = dataType,
                    .m_dataLength = data.length()
                },data);


    auto msg = receiveMessage(dataResponse);

    if(msg.m_type != LenovoLegionDaemon::MessageHeader::SET_DATA_RESPONSE)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::INVALID_MESSAGE,"Invalid set data request response message");
    }
    return dataResponse;
}
}
