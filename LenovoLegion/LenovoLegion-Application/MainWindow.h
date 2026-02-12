// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once
#include "TaskList.h"


#include <Core/ExceptionBuilder.h>
#include <Core/ApplicationModulesHandler.h>


#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

#include <QMainWindow>
#include <QString>




namespace Ui {
class MainWindow;
}


namespace  LenovoLegionGui {



class DataProviderManager;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    /*
     * Colors
     */
    static constexpr std::string_view VALUE_CHANGE_COLOR                       =    "#995e5e";
    static constexpr int TIMER_EVENT_IN_MS                                     =    500;


    DEFINE_EXCEPTION(MainWindow);

    enum ERROR_CODES : int {
        REQUEST_TIMEOUTED = 0
    };

private:

    /*
     * Daemon Connection Status
     */
    static constexpr std::string_view DAEMON_CONNECTION_STATUS_CONNECTED       =    "Daemon: Connected";
    static constexpr std::string_view DAEMON_CONNECTION_STATUS_DISCONNECTED    =    "Daemon: Disconnected";

public:

    explicit MainWindow(QWidget *parent = nullptr);

    virtual ~MainWindow();

    void addModules(std::unique_ptr<bj::framework::ApplicationModulesHandler>);


signals:
    
protected:

    void timerEvent(QTimerEvent *) override;
    void closeEvent(QCloseEvent *event) override;

public slots:

    void dameonConnectionStatus(bool isConnected);
    void daemonNotification(const legion::messages::Notification& msg);

private:

    void clientDisconnected();
    void clientConnected();
    void initializeUI();
    void clearUI();

private:

    Ui::MainWindow *ui;

    /*
     * Data Provider Manager
     */
    DataProviderManager*  m_dataProviderManager;

    /*
     * Timer ID
     */
    int m_timerId;

    /*
     * Async tasks list
     */
    TaskList m_asyncTasks;
};


}
