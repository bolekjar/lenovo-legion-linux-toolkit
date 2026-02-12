// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "MainWindow.h"
#include "AboutWindow.h"
#include "Settings.h"

#include <Core/ApplicationInterface.h>
#include <Core/ApplicationModulesLoader.h>

#include <QApplication>
#include <QSystemTrayIcon>

#include <memory>

namespace LenovoLegionGui {

class Application : public QApplication,
                    public bj::framework::ApplicationInterface
{
    Q_OBJECT
public:


public:

    Application(int &argc, char *argv[]);
    ~Application();

private:

    bool notify(QObject* receiver, QEvent* event) noexcept override;

    virtual void appRollBackImpl()                                          noexcept override;
    virtual void appStartImpl()                                                      override;
    virtual void appInitImpl(std::unique_ptr<ApplicationModulesHandler_T> modules)   override;
    virtual void appStopImpl()                                              noexcept override;
    virtual void appMainLoop()                                                       override;

    virtual void setLogingSeverityLevel(const bj::framework::Logger::SEVERITY_BITSET &severity) override;

    virtual const QDir  modulesPath() const                                          override;

private slots:

    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onExitActionTriggered();

    void onSettingChanged(LenovoLegionGui::ApplicationSettings::SettingType setting, bool value);
    void onThemeChanged(LenovoLegionGui::ApplicationSettings::ThemeType theme);

private:

    void loadSettings();
    void applyStartupSettings();
    void applyDebugLogging(bool enable);
    void applyTraceLogging(bool enable);
    void applyLogging(bool enableDebug, bool enableTrace);
    void applyTheme(ApplicationSettings::ThemeType theme);

    std::unique_ptr<MainWindow>      m_mainWindow;
    std::unique_ptr<AboutWindow>     m_aboutWindow;

    QSystemTrayIcon                 *m_trayIcon;
};



}
