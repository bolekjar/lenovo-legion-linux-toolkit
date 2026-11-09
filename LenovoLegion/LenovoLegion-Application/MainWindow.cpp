// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "MainWindow.h"

#include "Settings.h"
#include "DataProviderManager.h"

#include "Ui/AppSettings.h"
#include "Ui/AppState.h"
#include "Ui/DataSource.h"
#include "Ui/FieldModel.h"
#include "Ui/KeyboardLayout.h"
#include "Ui/PageSources.h"
#include "Ui/Theme.h"

#include <Core/Application.h>
#include <Core/LoggerHolder.h>

#include <QApplication>
#include <QCloseEvent>
#include <QIcon>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWidget>


namespace  LenovoLegionGui {

MainWindow::MainWindow(DataProviderManager &dataProviderManager, QWidget *parent) :
    QMainWindow(parent),
    m_dataProviderManager(dataProviderManager),
    m_view(nullptr),
    m_appState(nullptr)
{
    setWindowTitle("Lenovo Legion Linux Toolkit");
    setWindowIcon(QIcon(":/images/icons/LenovoLegionIco.png"));
    resize(1483,961);

    registerTypes();
    DataSource::setDataProvider(m_dataProviderManager.dataProvider());

    /*
     * Not connected until DataProviderManager reports the connection: no requests (no errors in the log)
     */
    DataSource::setConnected(false);

    /*
     * Reconnection to the daemon from the QML pages (Settings, status bar)
     */
    AppSettings::setReconnect([this]() {
        m_dataProviderManager.reconnectToDaemon();
    });

    m_view = new QQuickWidget(this);
    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);

    /*
     * Theme of the application, follows the theme selected in Settings
     */
    ApplicationSettings::ThemeType themeType = ApplicationSettings::ThemeType::NoTheme;
    ApplicationSettings::instance()->loadStylesheetTheme(themeType);
    auto* theme = new Theme(themeType,m_view);
    connect(ApplicationSettings::instance(),&ApplicationSettings::themeChanged,theme,&Theme::setTheme);

    m_appState = new AppState(QString::fromUtf8(bj::framework::Application::app_version),
                              [this]() { m_dataProviderManager.reconnectToDaemon(); },
                              []() { qApp->quit(); },
                              m_view);

    m_view->rootContext()->setContextProperty("theme",theme);
    m_view->rootContext()->setContextProperty("appSettings",new AppSettings(m_view));
    m_view->rootContext()->setContextProperty("appState",m_appState);

    connect(m_view,&QQuickWidget::statusChanged,this,[this](QQuickWidget::Status status) {
        if(status == QQuickWidget::Error)
        {
            for(const auto& error : m_view->errors())
            {
                LOG_E(QString("QML window: ").append(error.toString()));
            }
        }
    });

    m_view->setSource(QUrl("qrc:/ui/Shell.qml"));
    setCentralWidget(m_view);

    /*
     * Connect data provider manager events
     */
    connect(&m_dataProviderManager,&DataProviderManager::daemonNotification,this,&MainWindow::daemonNotification);
    connect(&m_dataProviderManager,&DataProviderManager::dameonConnectionStatus,this,&MainWindow::dameonConnectionStatus);
}

MainWindow::~MainWindow()
{
    AppSettings::setReconnect({});

    /*
     * Data sources of the pages are deleted with the view (they wait for their pending requests)
     */
    delete m_view;
    m_view = nullptr;
}

void MainWindow::addModules(std::unique_ptr<bj::framework::ApplicationModulesHandler>)
{}

void MainWindow::showAbout()
{
    show();
    raise();
    activateWindow();
    emit m_appState->aboutRequested();
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
    LOG_D(QString("MainWindow: daemon ").append(isConnected ? "connected" : "disconnected"));

    /*
     * The pages are created again when connected (Shell.qml), the other data sources read the data again
     */
    m_appState->setConnected(isConnected);
    DataSource::setConnected(isConnected);
    DataSource::reloadAll();
}

void MainWindow::daemonNotification(const legion::messages::Notification &msg)
{
    /*
     * Only the data types of the notification ((legion.ui.reload) in Notification.proto)
     */
    DataSource::reloadForNotification(msg.action());
}

void MainWindow::registerTypes()
{
    static bool registered = false;

    if(registered)
    {
        return;
    }

    registered = true;

    QQuickStyle::setStyle("Fusion");

    qmlRegisterType<DataSource>("LenovoLegion.Ui",1,0,"DataSource");
    qmlRegisterType<PageSources>("LenovoLegion.Ui",1,0,"PageSources");
    qmlRegisterType<KeyboardLayout>("LenovoLegion.Ui",1,0,"KeyboardLayout");
    qmlRegisterUncreatableType<FieldModel>("LenovoLegion.Ui",1,0,"FieldModel","FieldModel is created by DataSource.field()");
}

}
