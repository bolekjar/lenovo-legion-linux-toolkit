// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once


#include <Core/ExceptionBuilder.h>

#include <QObject>
#include <QLocalSocket>
#include <QFileSystemWatcher>



namespace LenovoLegionDaemon {

class ProtocolProcessorBase : public QObject
{
    Q_OBJECT
public:

    DEFINE_EXCEPTION(ProtocolProcessorBase);


    enum ERROR_CODES : int {
        CLIENT_POINTER_ERROR    = -1,
    };


public:

    ProtocolProcessorBase(QLocalSocket* clientSocket,QObject* parent = nullptr);
    virtual ~ProtocolProcessorBase();

    virtual void stop();
    virtual void start();


    void waitForExit();
    bool isRunning() const;


    /*
     * Message from the client. Exception disconnects only this client (dropClient).
     */
    virtual void readyReadHandler() = 0;

    /*
     * Client disconnected, socket is closed and clientDisconnected is emitted
     */
    virtual void disconnectedHandler();

signals:

    void clientDisconnected();

private slots:

    void readyReadHandlerSlot();

public:

    static void refuseConnection(QLocalSocket* clientSocket);

protected:

    /*
     * Disconnect the client after an error of its connection (e.g. invalid message), other clients are not affected
     */
    void dropClient(const QString& reason);

protected:

    QLocalSocket*            m_clientSocket;
};

}
