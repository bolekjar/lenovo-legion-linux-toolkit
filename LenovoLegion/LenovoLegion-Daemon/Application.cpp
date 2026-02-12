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


/*
 * SysFs Drivers
 */
#include "SysFsDriverCPU.h"
#include "SysFsDriverCPUXList.h"
#include "SysFsDriverCPUCore.h"
#include "SysFsDriverCPUAtom.h"
#include "SysFsDriverACPIPlatformProfile.h"
#include "SysFsDriverIntelPowercapRapl.h"
#include "SysFsDriverPowerSuplyBattery0.h"
#include "SysFSDriverLegionHWMon.h"
#include "SysFSDriverLegionFanMode.h"
#include "SysFSDriverLegionGameZone.h"
#include "SysFsDriverLegionOther.h"
#include "SysFsDriverLegionEvents.h"
#include "SysFSDriverLegionIntelMSR.h"
#include "SysFsDriverCPUInfo.h"
#include "SysFsDriverLegionMachineInformation.h"


/*
 * Data Providers
 */
#include "SysFsDataProviderHWMon.h"
#include "SysFsDataProviderCPUTopology.h"
#include "SysFsDataProviderPowerProfile.h"
#include "SysFsDataProviderBattery.h"
#include "SysFsDataProviderCPUPower.h"
#include "SysFsDataProviderGPUPower.h"
#include "SysFsDataProviderFanCurve.h"
#include "SysFsDataProviderFanOption.h"
#include "SysFsDataProviderCPUFrequency.h"
#include "SysFsDataProviderCPUOptions.h"
#include "SysFsDataProviderCPUSMT.h"
#include "SysFsDataProviderIntelMSR.h"
#include "SysFsDataProviderCPUInfo.h"
#include "SysFsDataProviderMachineInformation.h"
#include "SysFsDataProviderOther.h"
#include "SysFsDataProviderOtherGpuSwitch.h"

#include "DataProviderNvidiaNvml.h"
#include "DataProviderDaemonSettings.h"
#include "DataProviderRGBController.h"

#include "DaemonSettingsManager.h"


#include <Core/LoggerHolder.h>
#include <Core/Application.h>

#include <OS/Linux/Signal.h>

#include <QDir>

#include <signal.h>
#include <unistd.h>

namespace LenovoLegionDaemon {

Application::Application(int &argc, char *argv[]) :
    QCoreApplication(argc,argv),
    m_serverSocket(new QLocalServer(this)),
    m_serverSocketNotification(new QLocalServer(this)),
    m_sysFsDriverManager(new SysFsDriverManager(this)),
    m_dataProviderManager(new DataProviderManager(m_sysFsDriverManager,this)),
    m_protocolProcessor(nullptr),
    m_protocolProcessorNotification(nullptr)
{
    LoggerHolder::getInstance().init(QCoreApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::log_dir).append(QDir::separator()).append(bj::framework::Application::apps_names[1]).append(".log").toStdString());

    /*
     * Add SysFS Drivers
     */
    m_sysFsDriverManager->addDriver(new SysFSDriverLegionFanMode(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFSDriverLegionGameZone(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverLegionOther(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFSDriverLegionHWMon(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverCPU(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverCPUXList(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverCPUCore(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverCPUAtom(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverACPIPlatformProfile(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverIntelPowercapRapl(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverPowerSuplyBattery0(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverLegionEvents(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFSDriverLegionIntelMSR(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverCPUInfo(m_sysFsDriverManager));
    m_sysFsDriverManager->addDriver(new SysFsDriverLegionMachineInformation(m_sysFsDriverManager));


    /*
     * Add Data Providers
     */

    /*
     * SysFS Data Providers
     */
    m_dataProviderManager->addDataProvider(new SysFsDataProviderHWMon(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUTopology(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderPowerProfile(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderBattery(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUPower(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderGPUPower(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderFanCurve(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderFanOption(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUFrequency(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUOptions(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUSMT(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderIntelMSR(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderCPUInfo(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderMachineInformation(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderOther(m_sysFsDriverManager,m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new SysFsDataProviderOtherGpuSwitch(m_sysFsDriverManager,m_dataProviderManager));

    m_dataProviderManager->addDataProvider(new DataProviderNvidiaNvml(m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new DataProviderDaemonSettings(m_dataProviderManager));
    m_dataProviderManager->addDataProvider(new DataProviderRGBController(m_dataProviderManager));
}

void Application::appRollBackImpl() noexcept
{}

void Application::appStartImpl()
{}

void Application::appInitImpl(std::unique_ptr<ApplicationModulesHandler_T>)
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
     * Set signal handler
     */
    bj::framework::Signal::SetSignalHandler(SIGINT,std::bind(&Application::signalEventHandler,this,std::placeholders::_1));
    bj::framework::Signal::SetSignalHandler(SIGTERM,std::bind(&Application::signalEventHandler,this,std::placeholders::_1));


    /*
     * Init SysFs drivers
     */
    m_sysFsDriverManager->initDrivers();


    /*
     * Init Data Providers
     */
    m_dataProviderManager->initDataProviders();


    /*
     * Connect SysFs driver manager events to Data Provider Manager
     */
    connect(m_sysFsDriverManager,&SysFsDriverManager::kernelEvent,m_dataProviderManager,&DataProviderManager::kernelEventHandler);


    /*
     * Load settings
     */
    DaemonSettingsManager::getInstance().loadAllSettings(m_dataProviderManager);

    /*
     * Start Server
     */
    connect(m_serverSocket,&QLocalServer::newConnection,this,&Application::newConnectionHandler);

    m_serverSocket->setSocketOptions(QLocalServer::WorldAccessOption);
    m_serverSocket->setMaxPendingConnections(1);

    m_serverSocket->listen(SOCKET_NAME);


    /*
     * Start notification server
     */
    connect(m_serverSocketNotification,&QLocalServer::newConnection,this,&Application::newConnectionNotificationHandler);

    m_serverSocketNotification->setSocketOptions(QLocalServer::WorldAccessOption);
    m_serverSocketNotification->setMaxPendingConnections(1);

    m_serverSocketNotification->listen(SOCKET_NAME_NOTIFICATION);


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


    /*
     * Remove SysFs drivers
     */
    m_sysFsDriverManager->cleanDrivers();


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

        if(m_protocolProcessorNotification != nullptr && m_protocolProcessorNotification->isRunning())
        {
            LOG_W("Other client is connected, refuse notification connection !");
            ProtocolProcessorBase::refuseConnection(newSocket);
            return;
        }


        deleteProtocolProcessors<ProtocolProcessorBase,ProtocolProcessorNotifier>(m_protocolProcessorNotification);


        // Create processor - if this throws, pointer stays null (safe)
        ProtocolProcessorBase* newProcessor = new ProtocolProcessorNotifier(m_sysFsDriverManager,m_dataProviderManager, newSocket, this);
        
        // Connect signals - if this throws, we need to clean up
        try {
            connect(m_sysFsDriverManager,&SysFsDriverManager::kernelEvent,dynamic_cast<ProtocolProcessorNotifier*>(newProcessor),&ProtocolProcessorNotifier::kernelEventHandler);
            connect(m_sysFsDriverManager,&SysFsDriverManager::moduleSubsystem,dynamic_cast<ProtocolProcessorNotifier*>(newProcessor),&ProtocolProcessorNotifier::moduleSubsystemHandler);
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
