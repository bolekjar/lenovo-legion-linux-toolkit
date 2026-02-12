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
    m_protocolProcessor(new ProtocolProcessor(this)),
    m_protocolProcessorNotifier(new ProtocolProcessorNotifier(this)),
    m_dataProvider(new DataProvider(m_protocolProcessor,this))
{
    connect(m_protocolProcessor,&ProtocolProcessor::connected,this,&DataProviderManager::protocolProcessorConnected);
    connect(m_protocolProcessor,&ProtocolProcessor::disconnected,this,&DataProviderManager::protocolProcessorDisconnectd);
    connect(m_protocolProcessorNotifier,&ProtocolProcessorNotifier::daemonNotification,this,&DataProviderManager::daemonNotificationSlot);
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
    m_protocolProcessor->reconnect();
    m_protocolProcessorNotifier->reconnect();
}

}
