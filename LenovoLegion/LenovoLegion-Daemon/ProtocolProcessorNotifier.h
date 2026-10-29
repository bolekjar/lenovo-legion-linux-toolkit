// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProviderManager.h"

#include "ProtocolProcessorBase.h"



namespace LenovoLegionDaemon {


class ProtocolProcessorNotifier : public ProtocolProcessorBase
{
    Q_OBJECT

public:

    enum ERROR_CODES : int {
        UNEXPECTED_MESSAGE = -1,
        WATCHED_PATH_ERROR = -2,
        SERIALIZE_ERROR    = -3
    };


public:

    ProtocolProcessorNotifier(DataProviderManager* dataProviderManger,QLocalSocket* clientSocket,QObject* parent);
    ~ProtocolProcessorNotifier();

    virtual void stop()  override;
    virtual void start() override;

private:

    virtual void readyReadHandler() override;
    virtual void disconnectedHandler() override;

signals:

    void clientDisconnected();

public slots:


    /*
     * Ready notification payload of a daemon module, sent as it is
     */
    void providerNotificationHandler(const QByteArray& payload);
public:

    static constexpr quint8  m_dataType = 0;

private:

    DataProviderManager*    m_dataProviderManger;
};

}

