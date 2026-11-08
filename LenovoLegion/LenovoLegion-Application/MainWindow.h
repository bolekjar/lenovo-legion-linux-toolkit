// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>
#include <Core/ApplicationModulesHandler.h>


#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

#include <QMainWindow>
#include <QString>


class QQuickWidget;

namespace  LenovoLegionGui {


class AppState;
class DataProviderManager;

/*
 * Main window: the window of the generic GUI (Ui/qml/Shell.qml: navigation, pages, status bar with the daemon and the
 * battery, About) in a QQuickWidget. The window itself stays a widget for the system tray (hide on close).
 * Daemon notifications reload all data sources, the connection state is given to QML (AppState).
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(MainWindow);

    enum ERROR_CODES : int {
        REQUEST_TIMEOUTED = 0
    };

public:

    explicit MainWindow(DataProviderManager& dataProviderManager,QWidget *parent = nullptr);

    virtual ~MainWindow();

    void addModules(std::unique_ptr<bj::framework::ApplicationModulesHandler>);

    /*
     * Shows the window with the About dialog (tray menu)
     */
    void showAbout();

protected:

    void closeEvent(QCloseEvent *event) override;

public slots:

    void dameonConnectionStatus(bool isConnected);
    void daemonNotification(const legion::messages::Notification& msg);

private:

    /*
     * QML types of the generic GUI
     */
    static void registerTypes();

private:

    /*
     * Data Provider Manager
     */
    DataProviderManager&  m_dataProviderManager;

    QQuickWidget*         m_view;
    AppState*             m_appState;
};


}
