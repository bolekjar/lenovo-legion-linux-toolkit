// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ApplicationInterface.h>
#include <Core/ExceptionBuilder.h>


#include <QLocalServer>
#include <QLocalSocket>

#include <QCoreApplication>


namespace LenovoLegionDaemon {

class ProtocolProcessorBase;
class DataProviderManager;
class SysFsDriverManager;

class Application : public QCoreApplication,
                    public bj::framework::ApplicationInterface
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(Application);

    enum ERROR_CODES : int {
        NOT_ROOT   = -1,
        SOCKET_DIR_ERROR = -3,
        SIGNAL_ERROR = -4,
        SOCKET_LISTEN_ERROR = -5
    };

public:

    /*
     * Directory of the sockets (daemon and GUI): short fixed path, a socket path must fit sun_path (107 bytes),
     * a path in the directory of the application can be too long. Created by systemd (RuntimeDirectory of the
     * service) or by the daemon, removed on reboot.
     */
    static constexpr const char* const  SOCKET_DIR                   = "/run/lenovo-legion/";
    static constexpr const char* const  SOCKET_NAME                  = "lenovo_legion_daemon.socket";
    static constexpr const char* const  SOCKET_NAME_NOTIFICATION     = "lenovo_legion_daemon_notification.socket";
    static constexpr const char* const  SOCKET_GROUP                 = "legion";

public:

    Application(int &argc, char *argv[]);

private:

    bool notify(QObject* receiver, QEvent* event) override;

    virtual void appRollBackImpl()                                          noexcept override;
    virtual void appStartImpl()                                                      override;
    virtual void appInitImpl(std::unique_ptr<ApplicationModulesHandler_T> modules)   override;
    virtual void appStopImpl()                                              noexcept override;
    virtual void appMainLoop()                                                       override;

    virtual void setLogingSeverityLevel(const bj::framework::Logger::SEVERITY_BITSET& severity) override;



    virtual const QDir  modulesPath() const                                          override;


private slots:

    /*
     * New connnectios handler
     */
    void newConnectionHandler();

    /*
     * New connnectios notification handler
     */
    void newConnectionNotificationHandler();

private:

    /*
     * Unix signal handler, called from the event loop (see bj::framework::Signal)
     */
    void signalEventHandler(int signal) noexcept;

    /*
     * Create SOCKET_DIR if missing and check it is writable only by root
     */
    bool prepareSocketDir() const;

    /*
     * Start server socket accessible only by root and SOCKET_GROUP members, throws on failure
     */
    void startServer(QLocalServer* server,const char* name);

    /*
     * Check client credentials, allowed is root or SOCKET_GROUP member
     */
    bool isClientAuthorized(QLocalSocket* socket) const;

    /*
     * Accept the pending connection of the server, only one client per server, processor is deleted when the client disconnects.
     * Failure is logged and the connection is dropped, the daemon keeps running.
     */
    template<class Processor>
    void acceptConnection(QLocalServer* server,ProtocolProcessorBase* &protocolProcessor,const QString& name);

    /*
     * Stop and delete the processor (deferred, safe also from a slot invoked by the processor itself).
     * Outside of the event loop call QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete).
     */
    void deleteProtocolProcessor(ProtocolProcessorBase* &protocolProcessor);

private:

    /*
     * Socket path, SOCKET_DIR is writable only by root and SOCKET_GROUP members
     */
    const QString                   m_socketsPath;


    /*
     * Connections
     */
    QLocalServer*                   m_serverSocket;
    QLocalServer*                   m_serverSocketNotification;


    /*
     * SysFs Driver Manager
     */
    SysFsDriverManager*             m_sysFsDriverManager;


    /*
     * Data Provider Manager
     */
    DataProviderManager*            m_dataProviderManager;


    /*
     * Processing of the server protocol part
     */
    ProtocolProcessorBase*         m_protocolProcessor;

    /*
     * Processing of the notification protocol part
     */
    ProtocolProcessorBase*         m_protocolProcessorNotification;

};



}
