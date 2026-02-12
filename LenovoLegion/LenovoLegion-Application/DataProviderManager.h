// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <DataProvider.h>


#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

namespace  LenovoLegionGui {

class ProtocolProcessor;
class ProtocolProcessorNotifier;


class DataProviderManager : public QObject
{
    Q_OBJECT

public:

    DataProviderManager(QObject *parent);


    DataProvider * dataProvider() const;
    
public slots:
    void reconnectToDaemon();

signals:

    void dameonConnectionStatus(bool isConnected);
    void daemonNotification(legion::messages::Notification msg);


private slots:

    void protocolProcessorConnected();
    void protocolProcessorDisconnectd();

    void daemonNotificationSlot(const legion::messages::Notification &msg);

private:

    ProtocolProcessor*          m_protocolProcessor;
    ProtocolProcessorNotifier*  m_protocolProcessorNotifier;
    DataProvider*               m_dataProvider;
};

}
