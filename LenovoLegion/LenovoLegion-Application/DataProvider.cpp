// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProvider.h"
#include "ProtocolProcessor.h"

namespace LenovoLegionGui {

DataProvider::DataProvider(ProtocolProcessor &protocolProcessor, QObject *parent) :
    QObject(parent),
    m_protocolProcessor(protocolProcessor)
{

}

QByteArray DataProvider::getData(quint8 dataType, const QByteArray &request) const
{
    RequestResult responseBytes;

    if(!QMetaObject::invokeMethod(&m_protocolProcessor,
                                  &ProtocolProcessor::getDataRequest,
                                  Qt::BlockingQueuedConnection,
                                  qReturnArg(responseBytes),
                                  dataType,
                                  request)) {
        THROW_EXCEPTION(exception_T, ERROR_CODES::INVOKE_ERROR, "Failed to invoke method ProtocolProcessor::getDataRequest");
    }

    if(responseBytes.status != RequestResult::OK) {
        throwRequestError(responseBytes);
    }

    return responseBytes.data;
}

QByteArray DataProvider::setData(quint8 dataType, const QByteArray &data) const
{
    RequestResult resultBytes;

    if(!QMetaObject::invokeMethod(&m_protocolProcessor,
                                  &ProtocolProcessor::setDataRequest,
                                  Qt::BlockingQueuedConnection,
                                  qReturnArg(resultBytes),
                                  dataType,
                                  data)) {
        THROW_EXCEPTION(exception_T, ERROR_CODES::INVOKE_ERROR, "Failed to invoke Method ProtocolProcessor::setDataRequest");
    }

    if(resultBytes.status != RequestResult::OK) {
        throwRequestError(resultBytes);
    }

    return resultBytes.data;
}

}
