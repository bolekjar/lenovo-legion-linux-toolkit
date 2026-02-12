// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "Application.h"
#include "DataProviderManager.h"
#include "ProtocolProcessor.h"
#include "ProtocolProcessorNotifier.h"
#include "ToolBarSettingsWidget.h"

#include <Core/LoggerHolder.h>
#include <Core/Application.h>


#include <QDir>
#include <QMessageBox>
#include <QMenu>
#include <QToolTip>
#include <QEvent>


namespace LenovoLegionGui {

Application::Application(int &argc, char *argv[]) : QApplication(argc,argv),
    m_trayIcon(new QSystemTrayIcon(this))
{
    LoggerHolder::getInstance().init(QApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::log_dir).append(QDir::separator()).append(bj::framework::Application::apps_names[0]).append(".log").toStdString());
}

Application::~Application()
{}

void Application::appRollBackImpl() noexcept
{}

void Application::appStartImpl()
{}

void Application::appInitImpl(std::unique_ptr<ApplicationModulesHandler_T>)
{
    /*
     * Init Gui
     */

    m_mainWindow  = std::make_unique<MainWindow>();
    m_aboutWindow = std::make_unique<AboutWindow>();;


    /*
     * Init tray icon
     */
    m_trayIcon->setIcon(QIcon(":/images/icons/LenovoLegionIco.png"));
    m_trayIcon->setToolTip("Lenovo Legion control application");
    m_trayIcon->setContextMenu(new QMenu(m_mainWindow.get()));
    m_trayIcon->contextMenu()->addAction("Lenovo Legion", m_mainWindow.get(), &MainWindow::show);
    m_trayIcon->contextMenu()->addSeparator();
    m_trayIcon->contextMenu()->addAction("About", m_aboutWindow.get(), &AboutWindow::show);
    m_trayIcon->contextMenu()->addAction("Quit", this, &Application::onExitActionTriggered);


    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &Application::trayIconActivated);
    
    /*
     * Connect to settings changes
     */
    connect(ApplicationSettings::instance(), &ApplicationSettings::settingChanged,
            this, &Application::onSettingChanged);
    connect(ApplicationSettings::instance(), &ApplicationSettings::themeChanged,
            this, &Application::onThemeChanged);


    
    /*
     * Load and apply settings
     */
    loadSettings();
    applyStartupSettings();
}

void Application::appStopImpl() noexcept
{
    /*
     * Disconnect from settings signals
     */
    disconnect(ApplicationSettings::instance(), &ApplicationSettings::settingChanged,
               this, &Application::onSettingChanged);
    disconnect(ApplicationSettings::instance(), &ApplicationSettings::themeChanged,
               this, &Application::onThemeChanged);
    
    m_aboutWindow->close();
    m_mainWindow->close();

    m_trayIcon->setVisible(false);
    m_trayIcon->contextMenu()->clear();
}

void Application::appMainLoop()
{
    LOG_I(QString("Application name=").append(bj::framework::Application::apps_names[0]).append(" version=").append(bj::framework::Application::app_version).append(" started!"));
    m_trayIcon->setVisible(true);
    exec();
    LOG_I("Application stopped!");
}

void Application::setLogingSeverityLevel(const bj::framework::Logger::SEVERITY_BITSET &severity)
{
    LoggerHolder::getInstance().setSeverity(severity);
}

const QDir Application::modulesPath() const
{
    return QDir(QApplication::applicationDirPath().append(QDir::separator()).append(bj::framework::Application::modules_dir).append(QDir::separator()));
}

void Application::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {

        if(m_mainWindow->isVisible())
        {
            m_mainWindow->hide();
        }
        else
        {
            m_mainWindow->show();
        }
    }
}

void Application::onExitActionTriggered()
{
    m_aboutWindow->close();
    m_mainWindow->close();
    m_trayIcon->setVisible(false);
    QApplication::exit(0);
}

void Application::loadSettings()
{
    // Settings are loaded automatically via ApplicationSettings singleton
}

void Application::applyStartupSettings()
{
    // Apply debug logging setting
    bool appDebugLogging = false;
    ApplicationSettings::instance()->loadAppDebugLogging(appDebugLogging);
    
    // Apply trace logging setting
    bool appTraceLogging = false;
    ApplicationSettings::instance()->loadAppTraceLogging(appTraceLogging);
    
    applyLogging(appDebugLogging, appTraceLogging);
    
    // Apply theme setting
    ApplicationSettings::ThemeType theme = ApplicationSettings::ThemeType::NoTheme;
    ApplicationSettings::instance()->loadStylesheetTheme(theme);
    applyTheme(theme);
    LOG_T(QString("Applied theme: ").append(QString::number(static_cast<int>(theme))));
    
    // Apply start minimized setting
    bool startMinimized = false;
    ApplicationSettings::instance()->loadStartMinimized(startMinimized);
    
    if (startMinimized) {
        LOG_T("Starting minimized to system tray");
        // Don't show main window on startup
    } else {
        m_mainWindow->show();
    }
}

void Application::applyDebugLogging(bool enable)
{
    bool traceLogging = false;
    ApplicationSettings::instance()->loadAppTraceLogging(traceLogging);
    applyLogging(enable, traceLogging);
}

void Application::applyTraceLogging(bool enable)
{
    bool debugLogging = false;
    ApplicationSettings::instance()->loadAppDebugLogging(debugLogging);
    applyLogging(debugLogging, enable);
}

void Application::applyLogging(bool enableDebug, bool enableTrace)
{
    bj::framework::Logger::SEVERITY_BITSET severity(
        (1 << bj::framework::Logger::SEVERITY::INFO)     |
        (1 << bj::framework::Logger::SEVERITY::WARNING)  |
        (1 << bj::framework::Logger::SEVERITY::ERROR)
    );
    
    if (enableDebug) {
        severity |= (1 << bj::framework::Logger::SEVERITY::DEBUG);
    }
    
    if (enableTrace) {
        severity |= (1 << bj::framework::Logger::SEVERITY::TRACE);
    }
    
    setLogingSeverityLevel(severity);
    
    LOG_T(QString("Logging updated - Debug: ").append(enableDebug ? "enabled" : "disabled")
          .append(", Trace: ").append(enableTrace ? "enabled" : "disabled"));
}

void Application::applyTheme(ApplicationSettings::ThemeType theme)
{
    setStyleSheet(ToolBarSettingsWidget::getStylesheetTheme(theme));
}

void Application::onSettingChanged(LenovoLegionGui::ApplicationSettings::SettingType setting, bool value)
{
    using SettingType = ApplicationSettings::SettingType;
    
    switch (setting) {
        case SettingType::StartMinimized:
            LOG_T(QString("Setting changed: StartMinimized = ").append(value ? "true" : "false"));
            // This affects next startup, no action needed now
            break;
            
        case SettingType::MinimizeToTray:
            LOG_T(QString("Setting changed: MinimizeToTray = ").append(value ? "true" : "false"));
            // This is handled in MainWindow::closeEvent
            break;
            
        case SettingType::AppDebugLogging:
            LOG_T(QString("Setting changed: AppDebugLogging = ").append(value ? "true" : "false"));
            applyDebugLogging(value);
            break;
            
        case SettingType::AppTraceLogging:
            LOG_T(QString("Setting changed: AppTraceLogging = ").append(value ? "true" : "false"));
            applyTraceLogging(value);
            break;
            
        case SettingType::StylesheetTheme:
            LOG_T(QString("Setting changed: StylesheetTheme = ").append(value ? "true" : "false"));
            // This is handled in ToolBarSettingsWidget::onStylesheetThemeChanged
            break;
        default:
            break;
    }
}

void Application::onThemeChanged(LenovoLegionGui::ApplicationSettings::ThemeType theme)
{
    applyTheme(theme);
}

bool Application::notify(QObject* receiver, QEvent* event) noexcept {
  try {

      // Handle tooltip window creation to enable translucent background for rounded corners
      if (event->type() == QEvent::Polish) {
          QWidget* widget = qobject_cast<QWidget*>(receiver);
          if (widget && widget->inherits("QTipLabel")) {
              widget->setAttribute(Qt::WA_TranslucentBackground, true);
          }
      }

      return QApplication::notify(receiver, event);
  }
  catch (const ProtocolProcessor::exception_T& ex)
  {
      LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
      if(m_mainWindow) {
          QMessageBox::critical(m_mainWindow.get(),"LenovoLegion Protocol error",ex.descriptionInfo().value().c_str());
      }
  }
  catch (const ProtocolProcessorNotifier::exception_T& ex)
  {
      LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
      if(m_mainWindow) {
          QMessageBox::critical(m_mainWindow.get(),"LenovoLegion Protocol notifier error",ex.descriptionInfo().value().c_str());
      }
  }
  catch(const bj::framework::exception::Exception& ex)
  {    
      LOG_E(bj::framework::exception::ExceptionBuilder::print(ex).c_str());
      if(m_mainWindow) {
          QMessageBox::critical(m_mainWindow.get(),"LenovoLegion Error",QString("Unknown error occurred, error description = \" ").append(ex.descriptionInfo().value().c_str()).append(" \""));
      }
  } catch (...) {
      LOG_E(bj::framework::exception::ExceptionBuilder::print(__FILE__,__FUNCTION__,__LINE__,1,"Unknown error !").c_str());
      if(m_mainWindow) {
          QMessageBox::critical(m_mainWindow.get(),"LenovoLegion Error","Unknown error occurred!");
      }
  }

  return true;
}

}
