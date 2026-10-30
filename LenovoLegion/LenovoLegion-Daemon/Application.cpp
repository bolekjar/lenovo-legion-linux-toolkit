// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include <Application.h>

#include "ProtocolProcessor.h"
#include "ProtocolProcessorNotifier.h"

#include "DataProviderManager.h"
#include "SysFsDriverManager.h"
#include "DaemonSettingsManager.h"

/*
 * Data providers of the framework, hardware is handled by daemon modules (.so in modules/, see ModulesDesign.md)
 */
#include "DataProviders/DataProviderDaemonSettings.h"
#include "DataProviders/DataProviderProfiles.h"

#include "Modules/ModuleLoader.h"


#include <Core/LoggerHolder.h>
#include <Core/Application.h>

#include <OS/Linux/Signal.h>

#include <QDir>

#include <algorithm>
#include <vector>

#include <signal.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>

namespace LenovoLegionDaemon {

Application::Application(int &argc, char *argv[]) :
    QCoreApplication(argc,argv),
    m_socketsPath(QCoreApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::run_dir).append(QDir::separator())),
    m_serverSocket(new QLocalServer(this)),
    m_serverSocketNotification(new QLocalServer(this)),
    m_sysFsDriverManager(new SysFsDriverManager(this)),
    m_dataProviderManager(new DataProviderManager(this)),
    m_protocolProcessor(nullptr),
    m_protocolProcessorNotification(nullptr)
{
    LoggerHolder::getInstance().init(QCoreApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::log_dir).append(QDir::separator()).append(bj::framework::Application::apps_names[1]).append(".log").toStdString());


    /*
     * Data providers of the framework, hardware data providers are daemon modules loaded in appInitImpl
     */
    m_dataProviderManager->addDataProvider(new DataProviderDaemonSettings(m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new DataProviderProfiles(m_dataProviderManager));
}

void Application::appRollBackImpl() noexcept
{
    /*
     * Initialization failed, remove sockets
     */
    m_serverSocket->close();
    m_serverSocketNotification->close();
}

void Application::appStartImpl()
{}

void Application::appInitImpl(std::unique_ptr<ApplicationModulesHandler_T> modules)
{
     LOG_I(QString("Application ").append(bj::framework::Application::apps_names[1]).append(" version=").append(bj::framework::Application::app_version).append(" is starting ..."));

    /*
     * Check if you are root
     */
    if(geteuid() != 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::NOT_ROOT,"You must be root to run this application!");
    }

    /*
     * Check socket directory
     */
    if(!prepareSocketDir())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::SOCKET_DIR_ERROR,std::string("Socket directory ").append(m_socketsPath.toStdString()).append(" is not usable !").c_str());
    }

    /*
     * Set signal handler
     */
    if(!bj::framework::Signal::SetSignalHandler(SIGINT,std::bind(&Application::signalEventHandler,this,std::placeholders::_1)) ||
       !bj::framework::Signal::SetSignalHandler(SIGTERM,std::bind(&Application::signalEventHandler,this,std::placeholders::_1)))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::SIGNAL_ERROR,std::string("Unable to set signal handler : ").append(strerror(errno)).c_str());
    }


    /*
     * Start servers before touching the hardware, fail fast if sockets are not usable.
     * Connections are accepted from the event loop, i.e. after the initialization is finished.
     */

    /*
     * Start Server
     */
    connect(m_serverSocket,&QLocalServer::newConnection,this,&Application::newConnectionHandler);

    startServer(m_serverSocket,QString(m_socketsPath).append(SOCKET_NAME).toStdString().c_str());


    /*
     * Start notification server
     */
    connect(m_serverSocketNotification,&QLocalServer::newConnection,this,&Application::newConnectionNotificationHandler);

    startServer(m_serverSocketNotification,QString(m_socketsPath).append(SOCKET_NAME_NOTIFICATION).toStdString().c_str());


    /*
     * Load daemon modules (.so) from modules/, registered as data providers
     */
    ModuleLoader::loadModules(*modules,m_sysFsDriverManager,m_dataProviderManager);


    /*
     * Init Data Providers
     */
    m_dataProviderManager->initDataProviders();


    /*
     * Load settings
     */
    DaemonSettingsManager::getInstance().loadAllSettings(m_dataProviderManager);
}

void Application::appStopImpl() noexcept
{
    LOG_I("Stopping application ... ");



    /*
     * Stop notification Protocol processor
     */
    deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessorNotifier>(m_protocolProcessorNotification);


    /*
     * Stop Protocol processors
     */
    deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessor>(m_protocolProcessor);

    /*
     * Event loop is not running anymore, process deferred deletes of protocol processors now
     */
    QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);

    /*
     * Stop Server
     */
    disconnect(m_serverSocket,&QLocalServer::newConnection,this,&Application::newConnectionHandler);
    m_serverSocket->close();

    /*
     * Stop notification server
     */
    disconnect(m_serverSocketNotification,&QLocalServer::newConnection,this,&Application::newConnectionNotificationHandler);
    m_serverSocketNotification->close();

    /*
     * Save settings
     */
    DaemonSettingsManager::getInstance().saveAllSettingsOnExit(m_dataProviderManager);

    /*
     * Remove Data Providers
     */
    m_dataProviderManager->cleanDataProviders();


    LOG_I(QString("Application ").append(bj::framework::Application::apps_names[1]).append(" version=").append(bj::framework::Application::app_version).append(" stopped !"));
}

void Application::appMainLoop()
{
    LOG_I("Application appMainLoop started!");
    exec();
    LOG_I("Application appMainLoop ended!");
}

void Application::setLogingSeverityLevel(const bj::framework::Logger::SEVERITY_BITSET &severity)
{
    LoggerHolder::getInstance().setSeverity(severity);
}

const QDir Application::modulesPath() const
{
    return QDir(QCoreApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::modules_dir).append(QDir::separator()));
}

void Application::newConnectionHandler()
{
    LOG_D("New connection !");

    try {
        QLocalSocket* newSocket = m_serverSocket->nextPendingConnection();
        if(!newSocket)
        {
            LOG_W("nextPendingConnection returned null!");
            return;
        }

        if(!isClientAuthorized(newSocket))
        {
            LOG_W("Unauthorized client, refuse connection !");
            ProtocolProcessorBase::refuseConnection(newSocket);
            return;
        }

        if(m_protocolProcessor != nullptr && m_protocolProcessor->isRunning())
        {
            LOG_W("Other client is connected, refuse connection !");
            ProtocolProcessorBase::refuseConnection(newSocket);
            return;
        }

        deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessor>(m_protocolProcessor);

        // Create processor - if this throws, pointer stays null (safe)
        ProtocolProcessorBase* newProcessor = new ProtocolProcessor(m_dataProviderManager, newSocket, this);
        
        // Connect signals - if this throws, we need to clean up
        try {
            connect(dynamic_cast<ProtocolProcessor*>(newProcessor),&ProtocolProcessor::clientDisconnected,this,&Application::connectionDisconnectedHandler);
            newProcessor->start();
            
            // Only assign to member if everything succeeded
            m_protocolProcessor = newProcessor;
        }
        catch(...) {
            // Clean up on failure
            delete newProcessor;
            throw;
        }
    }
    catch (ProtocolProcessor::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == ProtocolProcessorBase::CLIENT_POINTER_ERROR)
        {
            bj::framework::exception::ExceptionBuilder::print(ex);
        }
        else
        {
            LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
            QCoreApplication::exit(0);
        }

    } catch (...)
    {
        LOG_E("New connection unknown error !");
    }

}

void Application::newConnectionNotificationHandler()
{
    LOG_D("New notification connection !");

    try {
        QLocalSocket* newSocket = m_serverSocketNotification->nextPendingConnection();
        if(!newSocket)
        {
            LOG_W("nextPendingConnection returned null!");
            return;
        }

        if(!isClientAuthorized(newSocket))
        {
            LOG_W("Unauthorized client, refuse notification connection !");
            ProtocolProcessorBase::refuseConnection(newSocket);
            return;
        }

        if(m_protocolProcessorNotification != nullptr && m_protocolProcessorNotification->isRunning())
        {
            LOG_W("Other client is connected, refuse notification connection !");
            ProtocolProcessorBase::refuseConnection(newSocket);
            return;
        }


        deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessorNotifier>(m_protocolProcessorNotification);


        // Create processor - if this throws, pointer stays null (safe)
        ProtocolProcessorBase* newProcessor = new ProtocolProcessorNotifier(m_dataProviderManager, newSocket, this);
        
        // Connect signals - if this throws, we need to clean up
        try {
            connect(dynamic_cast<ProtocolProcessorNotifier*>(newProcessor),&ProtocolProcessorNotifier::clientDisconnected,this,&Application::connectionNotificationDisconnectedHandler);
            newProcessor->start();
            
            // Only assign to member if everything succeeded
            m_protocolProcessorNotification = newProcessor;
        }
        catch(...) {
            // Clean up on failure
            delete newProcessor;
            throw;
        }
    }
    catch (ProtocolProcessor::exception_T& ex)
    {
        if(ex.errcodeInfo().value() == ProtocolProcessorBase::CLIENT_POINTER_ERROR)
        {
            bj::framework::exception::ExceptionBuilder::print(ex);
        }
        else
        {
            LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
            QCoreApplication::exit(0);
        }

    } catch (...)
    {
        LOG_E("New notification connection unknown error !");
    }
}

void Application::connectionDisconnectedHandler()
{
    LOG_D("Client disconnected, stopping processor !");

    deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessor>(m_protocolProcessor);
}

void Application::connectionNotificationDisconnectedHandler()
{
    LOG_D("Client disconnected, stopping notification processor !");

    deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessorNotifier>(m_protocolProcessorNotification);
}


bool Application::prepareSocketDir() const
{
    struct stat st {};

    if(lstat(m_socketsPath.toStdString().c_str(),&st) != 0)
    {
        LOG_E(QString("Unable to stat socket directory ").append(m_socketsPath).append(" : ").append(strerror(errno)));
        return false;
    }

    if(!S_ISDIR(st.st_mode) || st.st_uid != 0 || (st.st_mode & (S_IWGRP | S_IWOTH)) != 0)
    {
        LOG_E(QString("Socket directory ").append(m_socketsPath).append(" must be a directory owned by root and writable only by root !"));
        return false;
    }

    return true;
}

void Application::startServer(QLocalServer *server, const char *name)
{
    /*
     * Socket is created as 0660 root:root, group is changed to SOCKET_GROUP afterwards.
     * If the group is missing, socket stays accessible only by root.
     */
    server->setSocketOptions(QLocalServer::UserAccessOption | QLocalServer::GroupAccessOption);
    server->setMaxPendingConnections(1);

    /*
     * Remove stale socket after crash, SOCKET_DIR is writable only by root
     */
    QLocalServer::removeServer(name);

    if(!server->listen(name))
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::SOCKET_LISTEN_ERROR,QString("Unable to listen on ").append(name).append(" : ").append(server->errorString()).toStdString().c_str());
    }

    const struct group* grp = getgrnam(SOCKET_GROUP);
    if(grp == nullptr)
    {
        LOG_E(QString("Group ").append(SOCKET_GROUP).append(" does not exist, socket ").append(server->fullServerName()).append(" is accessible only by root !"));
        return;
    }

    if(lchown(server->fullServerName().toLocal8Bit().constData(),0,grp->gr_gid) != 0)
    {
        LOG_E(QString("Unable to change group of socket ").append(server->fullServerName()).append(" : ").append(strerror(errno)));
    }
}

bool Application::isClientAuthorized(QLocalSocket *socket) const
{
    struct ucred cred {};
    socklen_t    len = sizeof(cred);

    if(getsockopt(static_cast<int>(socket->socketDescriptor()),SOL_SOCKET,SO_PEERCRED,&cred,&len) != 0)
    {
        LOG_W(QString("Unable to get client credentials : ").append(strerror(errno)));
        return false;
    }

    if(cred.uid == 0)
    {
        return true;
    }

    const struct group*  grp = getgrnam(SOCKET_GROUP);
    const struct passwd* pwd = getpwuid(cred.uid);

    if(grp == nullptr || pwd == nullptr)
    {
        LOG_W(QString("Client uid=").append(QString::number(cred.uid)).append(" is not authorized !"));
        return false;
    }

    if(cred.gid == grp->gr_gid)
    {
        return true;
    }

    const gid_t groupId  = grp->gr_gid;
    int         ngroups  = 0;

    getgrouplist(pwd->pw_name,pwd->pw_gid,nullptr,&ngroups);

    std::vector<gid_t> groups(static_cast<size_t>(ngroups));

    if(getgrouplist(pwd->pw_name,pwd->pw_gid,groups.data(),&ngroups) >= 0 &&
       std::find(groups.begin(),groups.begin() + ngroups,groupId) != groups.begin() + ngroups)
    {
        return true;
    }

    LOG_W(QString("Client uid=").append(QString::number(cred.uid)).append(" is not member of group ").append(SOCKET_GROUP).append(", not authorized !"));
    return false;
}

void Application::signalEventHandler(int signal) noexcept
{
    LOG_D(QString("Signal ").append(QString::number(signal)).append(" received!"));

    if(signal == SIGINT || signal == SIGTERM)
    {
        QCoreApplication::exit(0);
    }
}



bool Application::notify(QObject* receiver, QEvent* event) {
  try {
      return QCoreApplication::notify(receiver, event);
  }
  catch(const bj::framework::exception::Exception& ex)
  {
      LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
  } catch (...) {
      LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__FUNCTION__,__LINE__,1,"Unknown error !").c_str());
  }

  LOG_D("Application notify error, cleaning protocol processors !");

  deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessor>(m_protocolProcessor);
  deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessorNotifier>(m_protocolProcessorNotification);

  return false;
}

}
