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


    virtual void readyReadHandler() = 0;
    virtual void disconnectedHandler();

private slots:

    void readyReadHandlerSlot();
    void disconnectedHandlerSlot();

public:

    static void refuseConnection(QLocalSocket* clientSocket);

protected:

    QLocalSocket*            m_clientSocket;
};

}
