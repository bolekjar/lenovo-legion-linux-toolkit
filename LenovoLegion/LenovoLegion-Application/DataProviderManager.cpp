// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderManager.h"

#include "ProtocolProcessor.h"
#include "ProtocolProcessorNotifier.h"


#include <../LenovoLegion-Daemon/Application.h>


#include <Core/LoggerHolder.h>


namespace  LenovoLegionGui {


DataProviderManager::DataProviderManager(QObject *parent) : QObject(parent),
    m_protocolProcessor(nullptr),
    m_protocolProcessorNotifier(nullptr),
    m_dataProvider(nullptr),
    m_communicationThread(nullptr)
{}

DataProviderManager::~DataProviderManager()
{}

void DataProviderManager::init()
{
    try {

        m_protocolProcessor         = new ProtocolProcessor();
        m_protocolProcessorNotifier = new ProtocolProcessorNotifier();
        m_dataProvider              = new DataProvider(*m_protocolProcessor, this);
        m_communicationThread       = new QThread(this);

        m_protocolProcessor->moveToThread(m_communicationThread);
        m_protocolProcessorNotifier->moveToThread(m_communicationThread);

        connect(m_communicationThread, &QThread::started, m_protocolProcessor, &ProtocolProcessor::init);
        connect(m_communicationThread, &QThread::started, m_protocolProcessorNotifier, &ProtocolProcessorNotifier::init);


        connect(m_communicationThread, &QThread::finished, m_protocolProcessor, &ProtocolProcessor::cleanup);
        connect(m_communicationThread, &QThread::finished, m_protocolProcessorNotifier, &ProtocolProcessorNotifier::cleanup);
        connect(m_communicationThread, &QThread::finished, m_protocolProcessor, &QObject::deleteLater);
        connect(m_communicationThread, &QThread::finished, m_protocolProcessorNotifier, &QObject::deleteLater);


        connect(m_protocolProcessor,&ProtocolProcessor::connected,this,&DataProviderManager::protocolProcessorConnected);
        connect(m_protocolProcessor,&ProtocolProcessor::disconnected,this,&DataProviderManager::protocolProcessorDisconnectd);
        connect(m_protocolProcessorNotifier,&ProtocolProcessorNotifier::daemonNotification,this,&DataProviderManager::daemonNotificationSlot);
        connect(m_protocolProcessor,&ProtocolProcessor::exceptionCaught,this,&DataProviderManager::exceptionCaughtSlot);
        connect(m_protocolProcessorNotifier,&ProtocolProcessorNotifier::exceptionCaught,this,&DataProviderManager::exceptionCaughtSlot);

        m_communicationThread->start();
    } catch (...)
    {
        cleanup();
        throw;
    }
}

void DataProviderManager::cleanup() noexcept
{
    if (m_communicationThread && m_communicationThread->isRunning()) {
        m_communicationThread->requestInterruption();
        m_communicationThread->quit();
        m_communicationThread->wait();
    }

    if(m_communicationThread)
    {
        delete m_communicationThread;
    }

    if(m_dataProvider)
    {
        delete m_dataProvider;
    }
}

DataProvider *DataProviderManager::dataProvider() const { return m_dataProvider; }

void DataProviderManager::protocolProcessorConnected()
{
    emit dameonConnectionStatus(true);
}

void DataProviderManager::protocolProcessorDisconnectd()
{
    emit dameonConnectionStatus(false);
}

void DataProviderManager::daemonNotificationSlot(const legion::messages::Notification& msg)
{
    emit daemonNotification(msg);
}

void DataProviderManager::exceptionCaughtSlot(const QString &error)
{
    LOG_E(QString("DataProviderManager exception: ").append(error));

    THROW_EXCEPTION(exception_T, ERROR_CODES::PROTOCOL_PROCESSOR_ERROR,error.toStdString());
}

void DataProviderManager::reconnectToDaemon()
{
    LOG_T("Manual reconnect to daemon requested");

    QMetaObject::invokeMethod(m_protocolProcessor, &ProtocolProcessor::reconnect);
    QMetaObject::invokeMethod(m_protocolProcessorNotifier, &ProtocolProcessorNotifier::reconnect);
}

}
