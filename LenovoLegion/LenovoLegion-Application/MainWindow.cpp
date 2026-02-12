// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "Settings.h"
#include "Utils.h"
#include "BateryStatus.h"
#include "ToolBarHomeWidget.h"
#include "ToolBarHelpWidget.h"
#include "ToolBarInformationWidget.h"
#include "ToolBarProfilesWidget.h"
#include "ToolBarSettingsWidget.h"
#include "ToolBarKeyboardWidget.h"
#include "RGBControllerInterface.h"


#include "DataProviderManager.h"


#include <Core/LoggerHolder.h>

#include <QCloseEvent>

#include <functional>


#include "../LenovoLegion-PrepareBuild/RGBController.pb.h"
#include "DataProviderRGBController.h"

namespace  LenovoLegionGui {

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_dataProviderManager(new DataProviderManager(this)),
    m_timerId(-1)
{
    ui->setupUi(this);

    /*
     * Init toolbar
     */
    auto toolbarActionInitializer = [this](QAction * action,const QIcon& icon,const std::function< void ()>& syncActions,bool checked = false)
    {
        action->setCheckable(true);
        action->setIcon(icon);
        action->setChecked(checked);

        connect(action, &QAction::triggered,[this,syncActions,action](bool)
        {

            /*
             * Cleanup existing widgets
             */
            Utils::Layout::forAllLayoutsDo(*ui->toolBar_HBoxLayout,[](QLayoutItem& layout)
            {
                dynamic_cast<ToolBarWidget *>(layout.widget())->blockSignals(true);
                dynamic_cast<ToolBarWidget *>(layout.widget())->cleanup();
            });

            /*
             * Remove all widgets from toolBar HBoxLayout
             */
            Utils::Layout::removeAllLayoutWidgets(*ui->toolBar_HBoxLayout);

            /*
             * Set action checked state
             */
            for (auto cur : ui->toolBar->actions())
            {
                if(cur->text() == action->text())
                    cur->setChecked(true);
                else
                    cur->setChecked(false);
            }

            syncActions();
        });
    };


    ui->toolBar->addSeparator();
    toolbarActionInitializer(ui->toolBar->addAction("Home"),QIcon(":/images/icons/toolbar-home.png"),[this](){

        if(ui->label_DaemonStatus->text() != DAEMON_CONNECTION_STATUS_CONNECTED.data())
            return;

        ui->toolBar_HBoxLayout->addWidget((new ToolBarHomeWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    },true);

    toolbarActionInitializer(ui->toolBar->addAction("Profiles"),QIcon(":/images/icons/toolbar-user.png"),[this](){
        if(ui->label_DaemonStatus->text() != DAEMON_CONNECTION_STATUS_CONNECTED.data())
            return;
        ui->toolBar_HBoxLayout->addWidget((new ToolBarProfilesWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    });

    toolbarActionInitializer(ui->toolBar->addAction("Keyboard"),QIcon(":/images/icons/keyboard.png"),[this](){
        if(ui->label_DaemonStatus->text() != DAEMON_CONNECTION_STATUS_CONNECTED.data())
            return;
        ui->toolBar_HBoxLayout->addWidget((new ToolBarKeyboardWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    });


    ui->toolBar->addWidget([](){QWidget * spacer = new QWidget(); spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding); return spacer;}());

    toolbarActionInitializer(ui->toolBar->addAction("Settings"),QIcon(":/images/icons/toolbar-settings.png"),[this](){
        if(ui->label_DaemonStatus->text() != DAEMON_CONNECTION_STATUS_CONNECTED.data())
            return;
        ui->toolBar_HBoxLayout->addWidget((new ToolBarSettingsWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    });
    toolbarActionInitializer(ui->toolBar->addAction("Information"),QIcon(":/images/icons/toolbar-information.png"),[this](){

        if(ui->label_DaemonStatus->text() != DAEMON_CONNECTION_STATUS_CONNECTED.data())
            return;

        ui->toolBar_HBoxLayout->addWidget((new ToolBarInformationWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    });

    toolbarActionInitializer(ui->toolBar->addAction("Help"),QIcon(":/images/icons/toolbar-question.png"),[this](){

        ui->toolBar_HBoxLayout->addWidget((new ToolBarHelpWidget(m_dataProviderManager->dataProvider(),this))->initialize());
    });

    toolbarActionInitializer(ui->toolBar->addAction("Exit"),QIcon(":/images/icons/toolbar-door.png"),[](){
        qApp->quit();
    });

    ui->toolBar->addSeparator();



    /*
     * Initial daemon status
     */

    ui->label_DaemonStatus->setText(DAEMON_CONNECTION_STATUS_DISCONNECTED.data());

    /*
     * Connect data provider manager events
     */
    connect(m_dataProviderManager,&DataProviderManager::daemonNotification,this,&MainWindow::daemonNotification);
    connect(m_dataProviderManager,&DataProviderManager::dameonConnectionStatus,this,&MainWindow::dameonConnectionStatus);

    /*
     * Start timer
     */
    m_timerId = startTimer(TIMER_EVENT_IN_MS);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addModules(std::unique_ptr<bj::framework::ApplicationModulesHandler>)
{}

void MainWindow::timerEvent(QTimerEvent *event)
{
    m_asyncTasks.runAll();

    /*
     * Delegate timer event to toolbar widgets
     */
    Utils::Layout::forAllLayoutsDo(*ui->toolBar_HBoxLayout,[event](QLayoutItem& layout)
    {
        dynamic_cast<ToolBarWidget *>(layout.widget())->mainWindowTimerEvent(event);
    });
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!event->spontaneous() || !isVisible())
        return;

    // Check MinimizeToTray setting
    bool minimizeToTray = false;
    ApplicationSettings::instance()->loadMinimizeToTray(minimizeToTray);
    
    if (minimizeToTray) {
        hide();
        event->ignore();
        LOG_T("Window minimized to system tray");
    } else {
        // Allow the window to close normally
        event->accept();
    }
}

void MainWindow::dameonConnectionStatus(bool isConnected)
{
    if(isConnected)
    {
        clientConnected();
    }
    else
    {
        clientDisconnected();
    }
}

void MainWindow::daemonNotification(const legion::messages::Notification &msg)
{
    switch (msg.action()) {
    case legion::messages::Notification::POWER_SUPPLY_BATTERY0_CHANGE:
    {
        if(ui->horizontalLayout_BateryStatus->itemAt(0) != nullptr)
        {
            dynamic_cast<BateryStatus*>(ui->horizontalLayout_BateryStatus->itemAt(0)->widget())->refresh();
        }
    }
    break;
    default:
        break;
    }


    Utils::Layout::forAllLayoutsDo(*ui->toolBar_HBoxLayout,[&msg](QLayoutItem& layout)
                                      {
                                          dynamic_cast<ToolBarWidget *>(layout.widget())->dataProviderEvent(msg);
                                      });
}

void MainWindow::clientDisconnected()
{
    LOG_D("MainWindow: Client disconnected from daemon");

    if(m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
    }

    /*
     * Set status
     */
    ui->label_DaemonStatus->setText(DAEMON_CONNECTION_STATUS_DISCONNECTED.data());

    /*
     *
     * Clear UI
     */
    clearUI();
}

void MainWindow::clientConnected()
{
    LOG_D("MainWindow: Client connected to daemon");

    if(m_timerId == -1)
    {
       m_timerId = startTimer(TIMER_EVENT_IN_MS);
    }

    /*
     * Set status
     */
    ui->label_DaemonStatus->setText(DAEMON_CONNECTION_STATUS_CONNECTED.data());

    /*
     * Initialize UI
     */
    initializeUI();
}

void MainWindow::initializeUI()
{
    /*
     * Add battery status widget
     */
    ui->horizontalLayout_BateryStatus->addWidget(new BateryStatus(m_dataProviderManager->dataProvider(),this));


    /*
     * Triger toolBar action to initialize proper widget
     */
    for (auto action : ui->toolBar->actions())
    {
        if(action->isChecked())
        {
            action->trigger();
            break;
        }
    }
}



void MainWindow::clearUI()
{

    /*
     * Cleanup existing widgets
     */
    Utils::Layout::forAllLayoutsDo(*ui->toolBar_HBoxLayout,[](QLayoutItem& layout)
                                   {
                                       dynamic_cast<ToolBarWidget *>(layout.widget())->blockSignals(true);
                                       dynamic_cast<ToolBarWidget *>(layout.widget())->cleanup();
                                   });

    /*
     * Remove all widgets from toolBar HBoxLayout
     */
    Utils::Layout::removeAllLayoutWidgets(*ui->toolBar_HBoxLayout);


    /*
     * Remove battery status widget
     */
    Utils::Layout::removeAllLayoutWidgets(*ui->horizontalLayout_BateryStatus);

    /*
     * Triger toolBar action to initialize proper widget
     */
    for (auto action : ui->toolBar->actions())
    {
        if(action->isChecked())
        {
            action->trigger();
            break;
        }
    }
}

}
