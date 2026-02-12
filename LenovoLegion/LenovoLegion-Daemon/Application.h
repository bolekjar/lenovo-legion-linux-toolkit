// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ApplicationInterface.h>
#include <Core/ApplicationModulesLoader.h>
#include <Core/ExceptionBuilder.h>


#include <QLocalServer>

#include <QCoreApplication>


namespace LenovoLegionDaemon {

class SysFsStructure;
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
        SAVE_ERROR = -2
    };

public:

    static constexpr const char* const  SOCKET_NAME                  = "LenovoLegionDaemonSocket";
    static constexpr const char* const  SOCKET_NAME_NOTIFICATION     = "LenovoLegionDaemonSocketNotifycation";

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


    /*
     * Connection disconnection handler
     */
    void connectionDisconnectedHandler();


    /*
     * Notification connection disconnection handler
     */
    void connectionNotificationDisconnectedHandler();

private:

    void signalEventHandler(int signal) noexcept;

    template<class T,class U>
    void deleteProtocolProcessors(T* &protocolProcessor)
    {
        if(protocolProcessor != nullptr)
        {
            // Disconnect ALL signals/slots FIRST to prevent queued signals from delivering
            disconnect(dynamic_cast<U*>(protocolProcessor), nullptr, nullptr, nullptr);
            
            // Block signals immediately (additional safety)
            protocolProcessor->blockSignals(true);

            // Stop the processor (closes socket, prevents new events)
            protocolProcessor->stop();

            // Remove ALL pending events for this object from the event queue
            // This prevents re-entrant calls during deletion
            QCoreApplication::removePostedEvents(protocolProcessor);
            
            // Process only deferred delete events for child objects (like socket)
            QCoreApplication::sendPostedEvents(protocolProcessor, QEvent::DeferredDelete);

            // Now safe to delete
            delete protocolProcessor;
            protocolProcessor = nullptr;
        }
    };

private:

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
