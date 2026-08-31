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
    m_protocolProcessor(new ProtocolProcessor()),
    m_protocolProcessorNotifier(new ProtocolProcessorNotifier()),
    m_dataProvider(new DataProvider(*m_protocolProcessor, this)),
    m_commThread(new QThread(this))
{
    m_protocolProcessor->moveToThread(m_commThread);
    m_protocolProcessorNotifier->moveToThread(m_commThread);

    connect(m_commThread, &QThread::started, m_protocolProcessor, &ProtocolProcessor::init);
    connect(m_commThread, &QThread::started, m_protocolProcessorNotifier, &ProtocolProcessorNotifier::init);


    connect(m_commThread, &QThread::finished, m_protocolProcessor, &ProtocolProcessor::cleanup);
    connect(m_commThread, &QThread::finished, m_protocolProcessorNotifier, &ProtocolProcessorNotifier::cleanup);
    connect(m_commThread, &QThread::finished, m_protocolProcessor, &QObject::deleteLater);
    connect(m_commThread, &QThread::finished, m_protocolProcessorNotifier, &QObject::deleteLater);


    connect(m_protocolProcessor,&ProtocolProcessor::connected,this,&DataProviderManager::protocolProcessorConnected);
    connect(m_protocolProcessor,&ProtocolProcessor::disconnected,this,&DataProviderManager::protocolProcessorDisconnectd);
    connect(m_protocolProcessorNotifier,&ProtocolProcessorNotifier::daemonNotification,this,&DataProviderManager::daemonNotificationSlot);
}

DataProviderManager::~DataProviderManager()
{}

void DataProviderManager::init()
{
    m_commThread->start();
}

void DataProviderManager::cleanup() noexcept
{
    if (m_commThread && m_commThread->isRunning()) {
        m_commThread->quit();
        m_commThread->wait();
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

void DataProviderManager::reconnectToDaemon()
{
    LOG_T("Manual reconnect to daemon requested");

    QMetaObject::invokeMethod(m_protocolProcessor, &ProtocolProcessor::reconnect);
    QMetaObject::invokeMethod(m_protocolProcessorNotifier, &ProtocolProcessorNotifier::reconnect);
}

}
