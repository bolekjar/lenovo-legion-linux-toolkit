#include "ToolBarSettingsWidget.h"
#include "ui_ToolBarSettingsWidget.h"

#include "Settings.h"

#include "Core/LoggerHolder.h"

#include "DataProviderManager.h"
#include "Utils.h"
#include "WidgetMessage.h"



#include "../LenovoLegion-Daemon/DataProviderDaemonSettings.h"
#include "../LenovoLegion-PrepareBuild/DaemonSettings.pb.h"


#include <QApplication>
#include <QMessageBox>
#include <QPixmap>
#include <QAbstractItemView>


namespace LenovoLegionGui {


ToolBarSettingsWidget::ToolBarSettingsWidget(DataProvider* dataProvider,QWidget *parent)
    : ToolBarWidget(dataProvider,parent)
    , ui(new Ui::ToolBarSettingsWidget)
{

    ui->setupUi(this);

    ui->comboBox_StylesheetTheme->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_StylesheetTheme->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    /*
     * Initialize actions map
     */

    m_defaultActionsMap["add"].push_back([]() {
        // Settings initialization
        LOG_D("ToolBarSettingsWidget initialized");
    });


    m_defaultActionsMap["remove"].push_back([]() {
        // Settings cleanup if needed
    });

    m_defaultActionsMap["refresh"].push_back([]() {
        // Refresh settings if needed
    });

    // Load settings from file
    loadSettings();

    // Connect signals
    connectSignals();

    Utils::Task::insertTasksBack(m_asyncTasks,m_defaultActionsMap["add"]);
}

void ToolBarSettingsWidget::dataProviderEvent(const legion::messages::Notification &event)
{
    // Settings widget doesn't need to react to most notifications
    Q_UNUSED(event);
}

void ToolBarSettingsWidget::cleanup()
{
    ToolBarWidget::cleanup();
}

ToolBarSettingsWidget::~ToolBarSettingsWidget()
{
    delete ui;
}

void ToolBarSettingsWidget::widgetEvent(const WidgetMessage &)
{}

void ToolBarSettingsWidget::onResetToDefaults()
{
    QMessageBox msgBox(this);
    QPixmap dialogIcon(":/images/icons/dialog.png");
    msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setWindowTitle("Reset to Defaults");
    msgBox.setText("Are you sure you want to reset all settings to their default values?");
    msgBox.setInformativeText("This action cannot be undone.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if (msgBox.exec() == QMessageBox::Yes) {
        resetAllSettingsToDefaults();
        LOG_D("All settings reset to defaults");
    }
}

void ToolBarSettingsWidget::resetAllSettingsToDefaults()
{
    ApplicationSettings::instance()->resetToDefaults();
    loadSettings();
    LOG_D("Settings reset to default values");
}

void ToolBarSettingsWidget::loadSettings()
{
    bool value;
    ApplicationSettings* settings = ApplicationSettings::instance();
    
    // Load application settings from INI
    settings->loadStartMinimized(value);
    ui->checkBox_StartMinimized->setChecked(value);
    
    settings->loadMinimizeToTray(value);
    ui->checkBox_MinimizeToTray->setChecked(value);
    
    settings->loadAppDebugLogging(value);
    ui->checkBox_AppDebugLogging->setChecked(value);
    
    settings->loadAppTraceLogging(value);
    ui->checkBox_AppTraceLogging->setChecked(value);
    
    // Load stylesheet theme
    ApplicationSettings::ThemeType theme;
    settings->loadStylesheetTheme(theme);
    ui->comboBox_StylesheetTheme->setCurrentIndex(static_cast<int>(theme));

    // Load daemon settings from daemon (not from app INI)
    loadDaemonSettings();
    
    LOG_D("Application settings loaded from file");
}

void ToolBarSettingsWidget::saveApplicationSettings()
{
    ApplicationSettings* settings = ApplicationSettings::instance();
    settings->saveStartMinimized(ui->checkBox_StartMinimized->isChecked())
            .saveMinimizeToTray(ui->checkBox_MinimizeToTray->isChecked())
            .saveAppDebugLogging(ui->checkBox_AppDebugLogging->isChecked())
            .saveAppTraceLogging(ui->checkBox_AppTraceLogging->isChecked())
            .saveStylesheetTheme(static_cast<ApplicationSettings::ThemeType>(ui->comboBox_StylesheetTheme->currentIndex()));
    
    LOG_D("Application settings saved to file");
}

void ToolBarSettingsWidget::connectSignals()
{
    // Connect application settings checkboxes (saved to app INI)
    connect(ui->checkBox_StartMinimized, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onApplicationSettingChanged);
    connect(ui->checkBox_MinimizeToTray, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onApplicationSettingChanged);
    connect(ui->checkBox_AppDebugLogging, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onApplicationSettingChanged);
    connect(ui->checkBox_AppTraceLogging, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onApplicationSettingChanged);
    
    // Connect theme combobox
    connect(ui->comboBox_StylesheetTheme, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ToolBarSettingsWidget::onStylesheetThemeChanged);
    
    // Connect daemon settings checkboxes (sent to daemon only)
    connect(ui->checkBox_ApplySavedSettingsOnStart, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onDaemonSettingChanged);
    connect(ui->checkBox_SaveSettingsOnDaemonExit, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onDaemonSettingChanged);
    connect(ui->checkBox_DebugLogging, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onDaemonSettingChanged);
    connect(ui->checkBox_TraceLogging, &QCheckBox::checkStateChanged, this, &ToolBarSettingsWidget::onDaemonSettingChanged);
    
    // Connect Reset to Defaults button
    connect(ui->pushButton_ResetDefaults, &QPushButton::clicked, this, &ToolBarSettingsWidget::onResetToDefaults);
    
    // Connect daemon buttons
    connect(ui->pushButton_SaveCurrentConfiguration, &QPushButton::clicked, this, &ToolBarSettingsWidget::onSaveCurrentConfiguration);
    connect(ui->pushButton_ReconnectDaemon, &QPushButton::clicked, this, &ToolBarSettingsWidget::onReconnectDaemon);
}

void ToolBarSettingsWidget::onApplicationSettingChanged()
{
    saveApplicationSettings();
}

void ToolBarSettingsWidget::onDaemonSettingChanged()
{
    sendDaemonSettingsToDaemon();
}

void ToolBarSettingsWidget::onReconnectDaemon()
{
    LOG_T("Reconnect to daemon button clicked");
    
    // Find DataProviderManager in parent hierarchy
    DataProviderManager* manager = nullptr;
    QWidget* parent = parentWidget();
    
    while (parent) {
        manager = parent->findChild<DataProviderManager*>();
        if (manager) {
            break;
        }
        parent = parent->parentWidget();
    }
    
    if (manager) {
        manager->reconnectToDaemon();
        LOG_T("Daemon reconnection initiated");
    } else {
        LOG_W("Could not find DataProviderManager to reconnect");
    }
}

void ToolBarSettingsWidget::loadDaemonSettings()
{
    legion::messages::DaemonSettings settings = m_dataProvider->getDataMessage<legion::messages::DaemonSettings>(
            LenovoLegionDaemon::DataProviderDaemonSettings::dataType);
        
    // Update UI from daemon settings
    ui->checkBox_ApplySavedSettingsOnStart->setChecked(settings.apply_settings_on_start());
    ui->checkBox_SaveSettingsOnDaemonExit->setChecked(settings.save_settings_on_exit());
    ui->checkBox_DebugLogging->setChecked(settings.debug_logging());
    ui->checkBox_TraceLogging->setChecked(settings.trace_logging());

    LOG_T("Daemon settings loaded from daemon");
}

void ToolBarSettingsWidget::sendDaemonSettingsToDaemon()
{
    legion::messages::DaemonSettings settings;
        
    // Set daemon configuration from UI
    settings.set_apply_settings_on_start(ui->checkBox_ApplySavedSettingsOnStart->isChecked());
    settings.set_save_settings_on_exit(ui->checkBox_SaveSettingsOnDaemonExit->isChecked());
    settings.set_debug_logging(ui->checkBox_DebugLogging->isChecked());
    settings.set_trace_logging(ui->checkBox_TraceLogging->isChecked());
        
    m_dataProvider->setDataMessage(LenovoLegionDaemon::DataProviderDaemonSettings::dataType, settings);

    LOG_T("Daemon settings sent to daemon");
}

void ToolBarSettingsWidget::onSaveCurrentConfiguration()
{
    LOG_T("Save Current Configuration button clicked");
    
    QMessageBox msgBox(this);
    QPixmap dialogIcon(":/images/icons/dialog.png");
    msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setWindowTitle("Save Configuration");
    msgBox.setText("Save the current hardware configuration?");
    msgBox.setInformativeText("This will save all current hardware settings (power profile, CPU settings, fan curves, etc.) to the daemon's configuration file.");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (msgBox.exec() == QMessageBox::Yes) {
        LOG_T("User confirmed: saving current configuration");
        
        // Send save_now command to daemon
        legion::messages::DaemonSettings settings;
        
        // Get current settings from UI
        settings.set_apply_settings_on_start(ui->checkBox_ApplySavedSettingsOnStart->isChecked());
        settings.set_save_settings_on_exit(ui->checkBox_SaveSettingsOnDaemonExit->isChecked());
        settings.set_debug_logging(ui->checkBox_DebugLogging->isChecked());
        settings.set_trace_logging(ui->checkBox_TraceLogging->isChecked());
        
        // Set the save_now command flag
        settings.set_save_now(true);
        
        // Send to daemon
        m_dataProvider->setDataMessage(LenovoLegionDaemon::DataProviderDaemonSettings::dataType, settings);
        
        LOG_D("Save current configuration command sent to daemon");

        QMessageBox msgBox(this);
        QPixmap dialogIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        msgBox.setWindowTitle("Save Configuration");
        msgBox.setText("Current configuration has been saved to the daemon's configuration file.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    } else {
        LOG_T("User cancelled configuration save");
    }
}

void ToolBarSettingsWidget::onStylesheetThemeChanged(int index)
{
    LOG_T(QString("Stylesheet theme changed to index: ") + QString::number(index));
    
    ApplicationSettings::ThemeType theme = static_cast<ApplicationSettings::ThemeType>(index);
    
    // Save the theme preference
    ApplicationSettings::instance()->saveStylesheetTheme(theme);
}

QString ToolBarSettingsWidget::getStylesheetTheme(ApplicationSettings::ThemeType theme)
{
    QString stylesheet;
    
    switch (theme) {
        case ApplicationSettings::ThemeType::NoTheme:
            stylesheet = "";
            LOG_D("Applying No Theme (default system theme)");
            break;
        case ApplicationSettings::ThemeType::WhiteTheme:
            stylesheet = getWhiteStylesheet();
            LOG_D("Applying Modern Blue (Simple) theme");
            break;
        case ApplicationSettings::ThemeType::GrayTheme:
            stylesheet = getGrayStylesheet();
            LOG_D("Applying Classic Gray theme");
            break;
        case ApplicationSettings::ThemeType::DarkTheme:
            stylesheet = getDarkStylesheet();
            LOG_D("Applying Modern Blue (Complete) theme");
            break;
    }

    return stylesheet;
}

QString ToolBarSettingsWidget::getWhiteStylesheet()
{
return R"(/* ===========================
   White Lenovo Legion Theme
   Clean and Bright Styling
   =========================== */

/* Basic Styling */
QWidget {
    background-color: #ffffff;
    border-radius: 12px;
    border: 0px;
    color: #2d2d30;
    font-family: "Segoe UI", "Ubuntu", sans-serif;
    font-size: 13px;
}

QMainWindow {
        background-color: #f5f5f5;
}

/* ===== QGroupBox ===== */
QGroupBox {
    border: 2px solid #e0e0e0;
    margin-top: 18px;
    font-size: 13px;
    font-weight: 500;
    border-radius: 12px;
        background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #fafafa, stop:1 #ffffff);
    padding: 8px;
}

QGroupBox::title {
    subcontrol-position: top left;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 6px 16px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #007acc, stop:1 #0096ff);
    color: #ffffff;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QGroupBox::indicator {
    width: 16px;
    height: 16px;
}

QGroupBox::indicator:unchecked {
    background-color: #ffffff;
    border: 2px solid #007acc;
    border-radius: 6px;
}

QGroupBox::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #007acc, stop:1 #00a8ff);
    border: 2px solid #00a8ff;
    border-radius: 6px;
}

/* ===== QPushButton ===== */
QPushButton {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #ffffff, stop:1 #f0f0f0);
        color: #007acc;
        border: 2px solid #007acc;
        border-radius: 12px;
        padding: 10px 20px;
        font-weight: bold;
        font-size: 13px;
        min-width: 90px;
        text-align: center;
        letter-spacing: 0.5px;
}

QPushButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #007acc, stop:1 #0096ff);
        border: 2px solid #007acc;
        color: #ffffff;
}

QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #005a9e, stop:1 #004578);
        border: 2px solid #004578;
        color: #ffffff;
        padding-top: 11px;
        padding-bottom: 9px;
}

QPushButton:checked {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #007acc, stop:1 #0096ff);
        border: 2px solid #007acc;
        color: #ffffff;
}

QPushButton:disabled {
        background: #e0e0e0;
        border: 2px solid #cccccc;
        color: #999999;
}

QPushButton:focus {
        border: 2px solid #00a8ff;
        outline: none;
}


/* ===== QToolBar ===== */
QToolBar {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #fafafa, stop:1 #f5f5f5);
    border-right: 2px solid #e0e0e0;
        border-radius: 0px;
}

QToolButton {
        background-color: transparent;
        margin: 5px 5px;
    width: 48px;
    height: 48px;
    border-radius: 10px;
    border: 2px solid transparent;
}

QToolButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f0f0f0);
    border: 2px solid #007acc;
}

QToolButton:checked {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    border: 2px solid #007acc;
}


/* ===== QLCDNumber ===== */
QLCDNumber {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
    border: 2px solid #007acc;
    border-radius: 12px;
    color: #000000;
    padding: 4px;
}





/* ===== QSlider ===== */
QSlider:horizontal {
    min-height: 32px;
}

QSlider::groove:horizontal {
        background-color: #e0e0e0;
        height: 6px;
    border-radius: 3px;
    border: 1px solid #cccccc;
}

QSlider::sub-page:horizontal {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                              stop:0 #007acc, stop:1 #00a8ff);
    border-radius: 3px;
}

QSlider::add-page:horizontal {
        background-color: #e0e0e0;
    border-radius: 3px;
}

QSlider::handle:horizontal {
        width: 22px;
    margin-top: -9px;
    margin-bottom: -9px;
    border-radius: 11px;
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #f0f0f0, stop:1 #e0e0e0);
    border: 2px solid #007acc;
}

QSlider::handle:horizontal:hover {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #ffffff, stop:1 #f0f0f0);
    border: 2px solid #00a8ff;
}

QSlider::sub-page:horizontal:disabled {
        background-color: #d0d0d0;
        border-color: #cccccc;
}

QSlider::add-page:horizontal:disabled {
        background-color: #f0f0f0;
        border-color: #e0e0e0;
}

QSlider::handle:horizontal:disabled {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(224, 224, 224, 255),
                              stop:0.7 rgba(208, 208, 208, 100));
        border: 2px solid #cccccc;
}

QSlider:vertical {
    min-width: 32px;
}

QSlider::groove:vertical {
    width: 6px;
    background-color: #e0e0e0;
    border-radius: 3px;
    border: 1px solid #cccccc;
}

QSlider::handle:vertical {
    height: 22px;
    margin-left: -9px;
    margin-right: -9px;
    border-radius: 11px;
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #f0f0f0, stop:1 #e0e0e0);
    border: 2px solid #007acc;
}

QSlider::handle:vertical:hover {
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #ffffff, stop:1 #f0f0f0);
    border: 2px solid #00a8ff;
}

QSlider::sub-page:vertical {
        background-color: #e0e0e0;
    border-radius: 3px;
}

QSlider::add-page:vertical {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #007acc, stop:1 #00a8ff);
    border-radius: 3px;
}

QSlider::sub-page:vertical:disabled {
        background-color: #f0f0f0;
        border-color: #e0e0e0;
}

QSlider::add-page:vertical:disabled {
        background-color: #d0d0d0;
        border-color: #cccccc;
}

QSlider::handle:vertical:disabled {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(224, 224, 224, 255),
                              stop:0.7 rgba(208, 208, 208, 100));
        border: 2px solid #cccccc;
}





/* ===== QTabWidget and QTabBar ===== */
QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #fafafa, stop:1 #ffffff);
    border: 2px solid #e0e0e0;
    border-radius: 12px;
    top: -1px;
    padding: 8px;
}

QTabWidget::tab-bar {
    alignment: left;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f0f0f0, stop:1 #e0e0e0);
    color: #2d2d30;
    border: 2px solid #cccccc;
    border-bottom: none;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 10px 24px;
    margin-right: 4px;
    font-size: 13px;
    font-weight: 500;
}

QTabBar::tab:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    color: #ffffff;
    border: 2px solid #007acc;
    border-bottom: none;
    font-weight: bold;
    padding-bottom: 12px;
}

QTabBar::tab:!selected {
    margin-top: 4px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
}

QTabBar::tab:!selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
    color: #007acc;
    border: 2px solid #007acc;
}

QTabBar::tab:first:!selected {
    margin-left: 0px;
}

QTabBar::tab:last {
    margin-right: 0;
}

QTabBar::tab:only-one {
    margin: 0;
}

QTabBar::tab:disabled {
    background-color: #f0f0f0;
    color: #999999;
    border: 2px solid #d0d0d0;
}

/* Vertical tabs (left/right) */
QTabBar::tab:left,
QTabBar::tab:right {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
}

QTabBar::tab:left {
    border-left: 2px solid #007acc;
    border-right: none;
    border-top-left-radius: 10px;
    border-bottom-left-radius: 10px;
    margin-bottom: 4px;
    margin-right: 0px;
}

QTabBar::tab:right {
    border-right: 2px solid #007acc;
    border-left: none;
    border-top-right-radius: 10px;
    border-bottom-right-radius: 10px;
    margin-bottom: 4px;
    margin-left: 0px;
}

QTabBar::tab:left:selected,
QTabBar::tab:right:selected {
    padding-bottom: 10px;
}

/* Close button on tabs */
QTabBar::close-button {
    image: url(:/images/icons/cross.png);
    subcontrol-position: right;
    border-radius: 6px;
    background-color: transparent;
    padding: 2px;
}

QTabBar::close-button:hover {
    background-color: #ff6b6b;
}

QTabBar::close-button:pressed {
    background-color: #ff4444;
}

/* ===== QToolTip ===== */
QToolTip {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    background-clip: border;
    color: #ffffff;
    border: 2px solid #00a8ff;
    border-radius: 8px;
    padding: 8px 14px;
    font-size: 13px;
    font-weight: 500;
    margin: 2px;
}


/* ===== QScrollBar ===== */
QScrollBar:vertical {
    border: none;
    background: #f5f5f5;
    width: 18px;
    border-radius: 9px;
}

QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #007acc, stop:1 #0096ff);
    min-height: 30px;
    border-radius: 7px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #005a9e, stop:1 #007acc);
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
}

QScrollBar:horizontal {
    border: none;
    background: #f5f5f5;
    height: 18px;
    border-radius: 9px;
}

QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    min-width: 30px;
    border-radius: 7px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::sub-line:horizontal {
        border: none;
        background: none;
}

QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QComboBox ===== */
QComboBox {
    background: transparent;
    combobox-popup: 0;
    border: 2px solid #007acc;
    border-radius: 10px;
    padding: 8px 12px;
    min-width: 6em;
    color: #2d2d30;
    font-size: 13px;
    selection-background-color: #007acc;
    selection-color: #ffffff;
}

QComboBox:hover {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QComboBox:focus {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QComboBox:on {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
    padding-top: 9px;
    padding-bottom: 7px;
}

QComboBox:disabled {
    background-color: #f0f0f0;
    border: 2px solid #d0d0d0;
    color: #999999;
}

QComboBox:editable {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
    color: #2d2d30;
}

QComboBox:editable:focus {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 32px;
    border-left: 2px solid #007acc;
    border-top-right-radius: 8px;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
}

QComboBox::drop-down:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QComboBox::drop-down:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QComboBox::drop-down:disabled {
    background-color: #d0d0d0;
    border-left: 2px solid #d0d0d0;
}

QComboBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 14px;
    height: 14px;
}

QComboBox::down-arrow:disabled {
    image: none;
}

QComboBox::down-arrow:on {
    top: 1px;
}

QComboBox QAbstractItemView {
    background-color: #ffffff;
    border: 2px solid #007acc;
    border-radius: 8px;
    padding: 4px;
    margin: 0px;
    selection-background-color: #007acc;
    selection-color: #ffffff;
    outline: none;
}

QComboBox QAbstractItemView::item {
    padding: 8px 12px;
    margin: 2px;
    border: none;
    border-radius: 6px;
    color: #2d2d30;
    min-height: 24px;
}

QComboBox QAbstractItemView::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #007acc, stop:1 #00a8ff);
    color: #ffffff;
    border-left: 4px solid #00a8ff;
    font-weight: bold;
}

QComboBox QAbstractItemView::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
    color: #007acc;
}

QComboBox QAbstractItemView::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #00a8ff, stop:1 #00c8ff);
    color: #ffffff;
    border-left: 4px solid #00c8ff;
    font-weight: bold;
}



/* ===== QCheckBox ===== */
QCheckBox {
    spacing: 12px;
    font-size: 13px;
    color: #2d2d30;
    padding: 4px;
}

QCheckBox:hover {
    color: #007acc;
}

QCheckBox:disabled {
    color: #999999;
}

QCheckBox::indicator {
    width: 20px;
    height: 20px;
    border-radius: 6px;
    border: 2px solid #007acc;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
}

QCheckBox::indicator:hover {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QCheckBox::indicator:pressed {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
}

QCheckBox::indicator:checked {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:checked:hover {
    border: 2px solid #00c8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QCheckBox::indicator:checked:pressed {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QCheckBox::indicator:unchecked:disabled {
    border: 2px solid #d0d0d0;
    background-color: #f0f0f0;
}

QCheckBox::indicator:checked:disabled {
    border: 2px solid #d0d0d0;
    background-color: #d0d0d0;
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:indeterminate {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
}

QCheckBox::indicator:indeterminate:hover {
    border: 2px solid #00c8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
}



/* ===== QRadioButton ===== */
QRadioButton {
    spacing: 12px;
    font-size: 13px;
    color: #2d2d30;
    padding: 4px;
}

QRadioButton:hover {
    color: #007acc;
}

QRadioButton:disabled {
    color: #999999;
}

QRadioButton::indicator {
    width: 20px;
    height: 20px;
    border-radius: 10px;
    border: 2px solid #007acc;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
}

QRadioButton::indicator:hover {
    border: 2px solid #00a8ff;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #ffffff,
                                stop:0.7 #f5f5f5,
                                stop:1 #f0f0f0);
}

QRadioButton::indicator:pressed {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
}

QRadioButton::indicator:checked {
    border: 2px solid #00a8ff;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #00a8ff,
                                stop:0.3 #007acc,
                                stop:0.6 #0096ff,
                                stop:1 #007acc);
}

QRadioButton::indicator:checked:hover {
    border: 2px solid #00c8ff;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #00c8ff,
                                stop:0.3 #00a8ff,
                                stop:0.6 #007acc,
                                stop:1 #0096ff);
}

QRadioButton::indicator:checked:pressed {
    border: 2px solid #00a8ff;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #007acc,
                                stop:0.5 #005a9e,
                                stop:1 #007acc);
}

QRadioButton::indicator:unchecked:disabled {
    border: 2px solid #d0d0d0;
    background-color: #f0f0f0;
}

QRadioButton::indicator:checked:disabled {
    border: 2px solid #d0d0d0;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #d0d0d0,
                                stop:0.5 #e0e0e0,
                                stop:1 #f0f0f0);
}

/* ===== QLineEdit ===== */
QLineEdit {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
    border: 2px solid #007acc;
    border-radius: 10px;
    padding: 8px 12px;
    color: #007acc;
    selection-background-color: #007acc;
    selection-color: #ffffff;
    font-size: 13px;
}

QLineEdit:hover {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QLineEdit:focus {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
    outline: none;
}

QLineEdit:disabled {
    background-color: #f0f0f0;
    border: 2px solid #d0d0d0;
    color: #999999;
}

QLineEdit[readOnly="true"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f5f5f5, stop:1 #f0f0f0);
    border: 2px solid #d0d0d0;
    color: #666666;
}

/* ===== QSpinBox ===== */
QSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
    border: 2px solid #007acc;
    border-radius: 10px;
    padding: 8px;
    color: #2d2d30;
    selection-background-color: #007acc;
    selection-color: #ffffff;
    font-size: 13px;
}

QSpinBox:hover {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QSpinBox:focus {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QSpinBox:disabled {
    background-color: #f0f0f0;
    border: 2px solid #d0d0d0;
    color: #999999;
}

QSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #007acc;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
}

QSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QSpinBox::up-button:disabled {
    background-color: #d0d0d0;
}

QSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::up-arrow:disabled {
    image: none;
}

QSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #007acc;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
}

QSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QSpinBox::down-button:disabled {
    background-color: #d0d0d0;
}

QSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QDoubleSpinBox ===== */
QDoubleSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #f5f5f5);
    border: 2px solid #007acc;
    border-radius: 10px;
    padding: 8px;
    color: #2d2d30;
    selection-background-color: #007acc;
    selection-color: #ffffff;
    font-size: 13px;
}

QDoubleSpinBox:hover {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QDoubleSpinBox:focus {
    border: 2px solid #00a8ff;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffffff, stop:1 #fafafa);
}

QDoubleSpinBox:disabled {
    background-color: #f0f0f0;
    border: 2px solid #d0d0d0;
    color: #999999;
}

QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #007acc;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
}

QDoubleSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QDoubleSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QDoubleSpinBox::up-button:disabled {
    background-color: #d0d0d0;
}

QDoubleSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::up-arrow:disabled {
    image: none;
}

QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #007acc;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
}

QDoubleSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QDoubleSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QDoubleSpinBox::down-button:disabled {
    background-color: #d0d0d0;
}

QDoubleSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QLabel ===== */
QLabel {
    background-color: transparent;
    color: #2d2d30;
    padding: 6px;
    font-size: 14px;
    font-weight: 500;
    border: 0px;
    border-radius: 6px;
}

QLabel[frameShape="6"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
    border: 2px solid #007acc;
    border-radius: 8px;
    padding: 8px 12px;
    color: #2d2d30;
    font-weight: 600;
}

QLabel[frameShape="1"] {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #005a9e, stop:1 #007acc);
    border: 2px solid #007acc;
    border-radius: 10px;
    padding: 10px 16px;
    color: #000000;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QLabel:disabled {
    color: #a0a0a0;
    background-color: #686868;
}

/* ===== QTableWidget ===== */
QTableWidget {
    background-color: #ffffff;
    border: 2px solid #007acc;
    border-radius: 12px;
    gridline-color: #e0e0e0;
    selection-background-color: transparent;
    selection-color: #ffffff;
    font-size: 13px;
    outline: none;
    padding: 5px;
}

QTableWidget::item {
    padding: 8px;
    border: none;
    background-color: #ffffff;
    color: #2d2d30;
    border-radius: 12px;
}

QTableWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #007acc, stop:1 #0096ff);
    color: #ffffff;
    font-weight: bold;
    border-radius: 12px;
}

QTableWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f0f0f0, stop:1 #e8e8e8);
    color: #007acc;
    border-radius: 12px;
}

QTableWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #00a8ff, stop:1 #00c8ff);
    color: #ffffff;
    border-radius: 12px;
}

QHeaderView::section {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    color: #ffffff;
    padding: 8px 12px;
    border: 1px solid #005a9e;
    border-radius: 6px;
    font-weight: bold;
    font-size: 13px;
    text-align: center;
}

QHeaderView::section:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QHeaderView::section:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QTableCornerButton::section {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    border: 1px solid #005a9e;
    border-radius: 6px;
}

QTableWidget QScrollBar:vertical {
    border: none;
    background: #f5f5f5;
    width: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #007acc, stop:1 #0096ff);
    min-height: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QTableWidget QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #005a9e, stop:1 #007acc);
}

QTableWidget QScrollBar::add-line:vertical, QTableWidget QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::add-page:vertical, QTableWidget QScrollBar::sub-page:vertical {
    background: none;
}

QTableWidget QScrollBar:horizontal {
    border: none;
    background: #f5f5f5;
    height: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #007acc, stop:1 #0096ff);
    min-width: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #00a8ff, stop:1 #00c8ff);
}

QTableWidget QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #005a9e, stop:1 #007acc);
}

QTableWidget QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::sub-line:horizontal {
    border: none;
    background: none;
}

QTableWidget QScrollBar::left-arrow:horizontal, QTableWidget QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QTableWidget QScrollBar::add-page:horizontal, QTableWidget QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QListWidget ===== */
QListWidget {
    background-color: #dadada;
    border: 2px solid qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #ffffff, stop:1 #fafafa);
    border-radius: 12px;
    padding: 2px;
    color: #000000;
    font-size: 12px;
    outline: none;
}

QListWidget::item {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #fafafa, stop:1 #f1f1f1);
    border: 1px solid #fbfbfb;
    border-radius: 8px;
    padding: 6px 12px;
    margin: 5px 3px;
    color: #000000;
    font-weight: 500;
}

QListWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #f1f1f1, stop:1 #e0e0e0);
    border: 1px solid #007acc;
    color: #000000;
    padding-left: 12px;
}

QListWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #e0e0e0, stop:1 #d0d0d0);
    border: 1px solid #007acc;
    color: #000000;
    padding-left: 12px;
}
QListWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #e0e0e0, stop:1 #d0d0d0);
    border: 1px solid #007acc;
    color: #000000;
    padding-left: 22px;
}

#CentralMainWindowHomeWidget {
    background-color: rgba(#fff, 0.5);
}

)";
}

QString ToolBarSettingsWidget::getGrayStylesheet()
{
    return R"(/* ===========================
   Gray Lenovo Legion Theme
   Professional Gray Styling
   =========================== */

/* Basic Styling */
QWidget {
        background-color: #808080;
    border-radius: 12px;
    border: 0px;
    color: #e0e0e0;
    font-family: "Segoe UI", "Ubuntu", sans-serif;
    font-size: 13px;
}

QMainWindow {
        background-color: #787878;
}

/* ===== QGroupBox ===== */
QGroupBox {
    border: 2px solid #606060;
    margin-top: 18px;
    font-size: 13px;
    font-weight: 500;
    border-radius: 12px;
        background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #787878, stop:1 #808080);
    padding: 8px;
}

QGroupBox::title {
    subcontrol-position: top left;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 6px 16px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #3f7950);
    color: #808080;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QGroupBox::indicator {
    width: 16px;
    height: 16px;
}

QGroupBox::indicator:unchecked {
    background-color: #808080;
    border: 2px solid #3c6844;
    border-radius: 6px;
}

QGroupBox::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #3c6844, stop:1 #4d8a5a);
    border: 2px solid #4d8a5a;
    border-radius: 6px;
}

/* ===== QPushButton ===== */
QPushButton {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #808080, stop:1 #686868);
        color: #3c6844;
        border: 2px solid #3c6844;
        border-radius: 12px;
        padding: 10px 20px;
        font-weight: bold;
        font-size: 13px;
        min-width: 90px;
        text-align: center;
        letter-spacing: 0.5px;
}

QPushButton:hover {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #3c6844, stop:1 #3f7950);
        border: 2px solid #3c6844;
        color: #808080;
}

QPushButton:pressed {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #2d4f33, stop:1 #1f3625);
        border: 2px solid #1f3625;
        color: #808080;
        padding-top: 11px;
        padding-bottom: 9px;
}

QPushButton:checked {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #3c6844, stop:1 #3f7950);
        border: 2px solid #3c6844;
        color: #808080;
}

QPushButton:disabled {
        background: #606060;
        border: 2px solid #4a4a4a;
        color: #a0a0a0;
}

QPushButton:focus {
        border: 2px solid #4d8a5a;
        outline: none;
}


/* ===== QToolBar ===== */
QToolBar {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #787878, stop:1 #707070);
    border-right: 2px solid #606060;
        border-radius: 0px;
}

QToolButton {
        background-color: transparent;
        margin: 5px 5px;
    width: 48px;
    height: 48px;
    border-radius: 10px;
    border: 2px solid transparent;
}

QToolButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #686868);
    border: 2px solid #3c6844;
}

QToolButton:checked {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    border: 2px solid #3c6844;
}


/* ===== QLCDNumber ===== */
QLCDNumber {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #737373, stop:1 #FFFFFF);
    border: 2px solid #3c6844;
    border-radius: 12px;
    color: #000000;
    padding: 4px;
}





/* ===== QSlider ===== */
QSlider:horizontal {
    min-height: 32px;
}

QSlider::groove:horizontal {
        background-color: #606060;
        height: 6px;
    border-radius: 3px;
    border: 1px solid #4a4a4a;
}

QSlider::sub-page:horizontal {
        background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                              stop:0 #3c6844, stop:1 #4d8a5a);
    border-radius: 3px;
}

QSlider::add-page:horizontal {
        background-color: #606060;
    border-radius: 3px;
}

QSlider::handle:horizontal {
        width: 22px;
    margin-top: -9px;
    margin-bottom: -9px;
    border-radius: 11px;
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #808080, stop:0.7 #686868, stop:1 #606060);
    border: 2px solid #3c6844;
}

QSlider::handle:horizontal:hover {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #808080, stop:0.7 #808080, stop:1 #686868);
    border: 2px solid #4d8a5a;
}

QSlider::sub-page:horizontal:disabled {
        background-color: #505050;
        border-color: #4a4a4a;
}

QSlider::add-page:horizontal:disabled {
        background-color: #686868;
        border-color: #606060;
}

QSlider::handle:horizontal:disabled {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(224, 224, 224, 255),
                              stop:0.7 rgba(208, 208, 208, 100));
        border: 2px solid #4a4a4a;
}

QSlider:vertical {
    min-width: 32px;
}

QSlider::groove:vertical {
    width: 6px;
    background-color: #606060;
    border-radius: 3px;
    border: 1px solid #4a4a4a;
}

QSlider::handle:vertical {
    height: 22px;
    margin-left: -9px;
    margin-right: -9px;
    border-radius: 11px;
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #808080, stop:0.7 #686868, stop:1 #606060);
    border: 2px solid #3c6844;
}

QSlider::handle:vertical:hover {
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #808080, stop:0.7 #808080, stop:1 #686868);
    border: 2px solid #4d8a5a;
}

QSlider::sub-page:vertical {
        background-color: #606060;
    border-radius: 3px;
}

QSlider::add-page:vertical {
        background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #3c6844, stop:1 #4d8a5a);
    border-radius: 3px;
}

QSlider::sub-page:vertical:disabled {
        background-color: #686868;
        border-color: #606060;
}

QSlider::add-page:vertical:disabled {
        background-color: #505050;
        border-color: #4a4a4a;
}

QSlider::handle:vertical:disabled {
        background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(224, 224, 224, 255),
                              stop:0.7 rgba(208, 208, 208, 100));
        border: 2px solid #4a4a4a;
}





/* ===== QTabWidget and QTabBar ===== */
QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #787878, stop:1 #808080);
    border: 2px solid #606060;
    border-radius: 12px;
    top: -1px;
    padding: 8px;
}

QTabWidget::tab-bar {
    alignment: left;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #686868, stop:1 #606060);
    color: #e0e0e0;
    border: 2px solid #4a4a4a;
    border-bottom: none;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 10px 24px;
    margin-right: 4px;
    font-size: 13px;
    font-weight: 500;
}

QTabBar::tab:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    color: #808080;
    border: 2px solid #3c6844;
    border-bottom: none;
    font-weight: bold;
    padding-bottom: 12px;
}

QTabBar::tab:!selected {
    margin-top: 4px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
}

QTabBar::tab:!selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    color: #3c6844;
    border: 2px solid #3c6844;
}

QTabBar::tab:first:!selected {
    margin-left: 0px;
}

QTabBar::tab:last {
    margin-right: 0;
}

QTabBar::tab:only-one {
    margin: 0;
}

QTabBar::tab:disabled {
    background-color: #686868;
    color: #a0a0a0;
    border: 2px solid #505050;
}

/* Vertical tabs (left/right) */
QTabBar::tab:left,
QTabBar::tab:right {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
}

QTabBar::tab:left {
    border-left: 2px solid #3c6844;
    border-right: none;
    border-top-left-radius: 10px;
    border-bottom-left-radius: 10px;
    margin-bottom: 4px;
    margin-right: 0px;
}

QTabBar::tab:right {
    border-right: 2px solid #3c6844;
    border-left: none;
    border-top-right-radius: 10px;
    border-bottom-right-radius: 10px;
    margin-bottom: 4px;
    margin-left: 0px;
}

QTabBar::tab:left:selected,
QTabBar::tab:right:selected {
    padding-bottom: 10px;
}

/* Close button on tabs */
QTabBar::close-button {
    image: url(:/images/icons/cross.png);
    subcontrol-position: right;
    border-radius: 6px;
    background-color: transparent;
    padding: 2px;
}

QTabBar::close-button:hover {
    background-color: #ff6b6b;
}

QTabBar::close-button:pressed {
    background-color: #ff4444;
}

/* ===== QToolTip ===== */
QToolTip {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    background-clip: border;
    color: #808080;
    border: 2px solid #4d8a5a;
    border-radius: 8px;
    padding: 8px 14px;
    font-size: 13px;
    font-weight: 500;
    margin: 2px;
}


/* ===== QScrollBar ===== */
QScrollBar:vertical {
    border: none;
    background: #707070;
    width: 18px;
    border-radius: 9px;
}

QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #3f7950);
    min-height: 30px;
    border-radius: 7px;
    margin: 2px;
}
QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
}

QScrollBar:horizontal {
    border: none;
    background: #707070;
    height: 18px;
    border-radius: 9px;
}

QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    min-width: 30px;
    border-radius: 7px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::sub-line:horizontal {
        border: none;
        background: none;
}

QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QComboBox ===== */
QComboBox {
    background: transparent;
    combobox-popup: 0;
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 8px 12px;
    min-width: 6em;
    color: #e0e0e0;
    font-size: 13px;
    selection-background-color: #3c6844;
    selection-color: #808080;
}

QComboBox:hover {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QComboBox:focus {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QComboBox:on {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
    padding-top: 9px;
    padding-bottom: 7px;
}

QComboBox:disabled {
    background-color: #686868;
    border: 2px solid #505050;
    color: #a0a0a0;
}

QComboBox:editable {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    color: #e0e0e0;
}

QComboBox:editable:focus {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 32px;
    border-left: 2px solid #3c6844;
    border-top-right-radius: 8px;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
}

QComboBox::drop-down:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QComboBox::drop-down:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QComboBox::drop-down:disabled {
    background-color: #505050;
    border-left: 2px solid #505050;
}

QComboBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 14px;
    height: 14px;
}

QComboBox::down-arrow:disabled {
    image: none;
}

QComboBox::down-arrow:on {
    top: 1px;
}

QComboBox QAbstractItemView {
    background-color: #787878;
    border: 2px solid #3c6844;
    border-radius: 8px;
    padding: 4px;
    margin: 0px;
    selection-background-color: #3c6844;
    selection-color: #808080;
    outline: none;
}

QComboBox QAbstractItemView::item {
    padding: 8px 12px;
    margin: 2px;
    border: none;
    border-radius: 6px;
    color: #e0e0e0;
    min-height: 24px;
}

QComboBox QAbstractItemView::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #4d8a5a);
    color: #808080;
    border-left: 4px solid #4d8a5a;
    font-weight: bold;
}

QComboBox QAbstractItemView::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
    color: #3c6844;
}

QComboBox QAbstractItemView::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #4d8a5a, stop:1 #66a876);
    color: #808080;
    border-left: 4px solid #66a876;
    font-weight: bold;
}



/* ===== QCheckBox ===== */
QCheckBox {
    spacing: 12px;
    font-size: 13px;
    color: #e0e0e0;
    padding: 4px;
}

QCheckBox:hover {
    color: #3c6844;
}

QCheckBox:disabled {
    color: #a0a0a0;
}

QCheckBox::indicator {
    width: 20px;
    height: 20px;
    border-radius: 6px;
    border: 2px solid #3c6844;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
}

QCheckBox::indicator:hover {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QCheckBox::indicator:pressed {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
}

QCheckBox::indicator:checked {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:checked:hover {
    border: 2px solid #66a876;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QCheckBox::indicator:checked:pressed {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QCheckBox::indicator:unchecked:disabled {
    border: 2px solid #505050;
    background-color: #686868;
}

QCheckBox::indicator:checked:disabled {
    border: 2px solid #505050;
    background-color: #505050;
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:indeterminate {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
}

QCheckBox::indicator:indeterminate:hover {
    border: 2px solid #66a876;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
}



/* ===== QRadioButton ===== */
QRadioButton {
    spacing: 12px;
    font-size: 13px;
    color: #e0e0e0;
    padding: 4px;
}


QRadioButton:hover {
    color: #3c6844;
}

QRadioButton:disabled {
    color: #a0a0a0;
}

QRadioButton::indicator {
    width: 20px;
    height: 20px;
    border-radius: 10px;
    border: 2px solid #3c6844;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #737373, stop:1 #FFFFFF);
}

QRadioButton::indicator:hover {
    border: 2px solid #4d8a5a;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #808080,
                                stop:0.7 #707070,
                                stop:1 #686868);
}

QRadioButton::indicator:pressed {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
}

QRadioButton::indicator:checked {
    border: 2px solid #4d8a5a;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #4d8a5a,
                                stop:0.3 #3c6844,
                                stop:0.6 #3f7950,
                                stop:1 #3c6844);
}

QRadioButton::indicator:checked:hover {
    border: 2px solid #66a876;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #66a876,
                                stop:0.3 #4d8a5a,
                                stop:0.6 #3c6844,
                                stop:1 #3f7950);
}

QRadioButton::indicator:checked:pressed {
    border: 2px solid #4d8a5a;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #3c6844,
                                stop:0.5 #2d4f33,
                                stop:1 #3c6844);
}

QRadioButton::indicator:unchecked:disabled {
    border: 2px solid #505050;
    background-color: #686868;
}

QRadioButton::indicator:checked:disabled {
    border: 2px solid #505050;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #505050,
                                stop:0.5 #606060,
                                stop:1 #686868);
}

/* ===== QLineEdit ===== */
QLineEdit {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 8px 12px;
    color: #3c6844;
    selection-background-color: #3c6844;
    selection-color: #808080;
    font-size: 13px;
}

QLineEdit:hover {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QLineEdit:focus {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
    outline: none;
}

QLineEdit:disabled {
    background-color: #686868;
    border: 2px solid #505050;
    color: #a0a0a0;
}

QLineEdit[readOnly="true"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #707070, stop:1 #686868);
    border: 2px solid #505050;
    color: #b0b0b0;
}

/* ===== QSpinBox ===== */
QSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 8px;
    color: #e0e0e0;
    selection-background-color: #3c6844;
    selection-color: #808080;
    font-size: 13px;
}

QSpinBox:hover {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QSpinBox:focus {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QSpinBox:disabled {
    background-color: #686868;
    border: 2px solid #505050;
    color: #a0a0a0;
}

QSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #3c6844;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
}

QSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QSpinBox::up-button:disabled {
    background-color: #505050;
}

QSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::up-arrow:disabled {
    image: none;
}

QSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #3c6844;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
}

QSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QSpinBox::down-button:disabled {
    background-color: #505050;
}

QSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QDoubleSpinBox ===== */
QDoubleSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 8px;
    color: #e0e0e0;
    selection-background-color: #3c6844;
    selection-color: #808080;
    font-size: 13px;
}

QDoubleSpinBox:hover {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QDoubleSpinBox:focus {
    border: 2px solid #4d8a5a;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #787878);
}

QDoubleSpinBox:disabled {
    background-color: #686868;
    border: 2px solid #505050;
    color: #a0a0a0;
}

QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #3c6844;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
}

QDoubleSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QDoubleSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QDoubleSpinBox::up-button:disabled {
    background-color: #505050;
}

QDoubleSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::up-arrow:disabled {
    image: none;
}

QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #3c6844;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
}

QDoubleSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QDoubleSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QDoubleSpinBox::down-button:disabled {
    background-color: #505050;
}

QDoubleSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QLabel ===== */
QLabel {
    background-color: transparent;
    color: #e0e0e0;
    padding: 6px;
    font-size: 14px;
    font-weight: 500;
    border: 0px;
    border-radius: 6px;
}

QLabel[frameShape="6"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    border: 2px solid #3c6844;
    border-radius: 8px;
    padding: 8px 12px;
    color: #e0e0e0;
    font-weight: 600;
}

QLabel[frameShape="1"] {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #4d8a5a);
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 10px 16px;
    color: #ffffff;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QLabel:disabled {
    color: #a0a0a0;
    background-color: #686868;
}

/* ===== QTableWidget ===== */
QTableWidget {
    background-color: #808080;
    border: 2px solid #3c6844;
    border-radius: 12px;
    gridline-color: #606060;
    selection-background-color: transparent;
    selection-color: #ffffff;
    font-size: 13px;
    outline: none;
    padding: 5px;
}

QTableWidget::item {
    padding: 8px;
    border: none;
    background-color: #808080;
    border-radius: 12px;
    color: #e0e0e0;
}

QTableWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #3f7950);
    color: #ffffff;
    border-radius: 12px;
    font-weight: bold;
}

QTableWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #686868, stop:1 #606060);
    color: #4d8a5a;
}

QTableWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #4d8a5a, stop:1 #66a876);
    border-radius: 12px;
    color: #ffffff;
}

QHeaderView::section {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    color: #ffffff;
    padding: 8px 12px;
    border: 1px solid #2d4f33;
    border-radius: 6px;
    font-weight: bold;
    font-size: 13px;
    text-align: center;
}

QHeaderView::section:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QHeaderView::section:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QTableCornerButton::section {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    border: 1px solid #2d4f33;
    border-radius: 6px;
}

QTableWidget QScrollBar:vertical {
    border: none;
    background: #707070;
    width: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #3f7950);
    min-height: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QTableWidget QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QTableWidget QScrollBar::add-line:vertical, QTableWidget QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::add-page:vertical, QTableWidget QScrollBar::sub-page:vertical {
    background: none;
}

QTableWidget QScrollBar:horizontal {
    border: none;
    background: #707070;
    height: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3c6844, stop:1 #3f7950);
    min-width: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4d8a5a, stop:1 #66a876);
}

QTableWidget QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d4f33, stop:1 #3c6844);
}

QTableWidget QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::sub-line:horizontal {
    border: none;
    background: none;
}

QTableWidget QScrollBar::left-arrow:horizontal, QTableWidget QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QTableWidget QScrollBar::add-page:horizontal, QTableWidget QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QListWidget ===== */
QListWidget {
    background-color: #808080;
    border: 2px solid qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #808080, stop:1 #707070);
    border-radius: 12px;
    padding: 2px;
    color: #ffffff;
    font-size: 12px;
    outline: none;
}

QListWidget::item {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #606060, stop:1 #505050);
    border-radius: 8px;
    padding: 6px 12px;
    margin: 5px 3px;
    color: #ffffff;
    font-weight: 500;
}

QListWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3f3f3f, stop:1 #505050);
    border: 1px solid #3c6844;
    color: #ffffff;
    padding-left: 12px;
}

QListWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3f3f3f, stop:1 #474747);
    border: 1px solid #3c6844;
    color: #ffffff;
    padding-left: 12px;
}
QListWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #4f4f4f, stop:1 #606060);
    border: 1px solid #3c6844;
    color: #ffffff;
    padding-left: 22px;
}

#CentralMainWindowHomeWidget {
    background-color: rgba(#fff, 0.5);
}
)";
}

QString ToolBarSettingsWidget::getDarkStylesheet()
{
    return R"(
/* ===========================
   Modern Lenovo Legion Theme
   Enhanced Readability and Styling
   =========================== */

/* Basic Styling */
QWidget {
	background-color: #2d2d30;
    border-radius: 12px;
    border: 0px;
    color: #e0e0e0;
    font-family: "Segoe UI", "Ubuntu", sans-serif;
    font-size: 13px;
}

QMainWindow {
	background-color: #252526;
}

/* ===== QGroupBox ===== */
QGroupBox {
    border: 2px solid #3e3e42;
    margin-top: 18px;
    font-size: 13px;
    font-weight: 500;
    border-radius: 12px;
	background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #3a3a3d, stop:1 #2d2d30);
    padding: 8px;
}

QGroupBox::title {
    subcontrol-position: top left;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 6px 16px;
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff8800, stop:1 #cc6600);
    color: #ffffff;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QGroupBox::indicator {
    width: 16px;
    height: 16px;
}

QGroupBox::indicator:unchecked {
    background-color: #2d2d30;
    border: 2px solid #ff8800;
    border-radius: 6px;
}

QGroupBox::indicator:checked {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #ff8800, stop:1 #ffaa33);
    border: 2px solid #ffaa33;
    border-radius: 6px;
}

/* ===== QPushButton ===== */
QPushButton {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #ff8800, stop:1 #cc6600);
	color: #ffffff;
	border: 2px solid #cc6600;
	border-radius: 12px;
	padding: 10px 20px;
	font-weight: bold;
	font-size: 13px;
	min-width: 90px;
	text-align: center;
	letter-spacing: 0.5px;
}

QPushButton:hover {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #ffaa33, stop:1 #ff8800);
	border: 2px solid #ffaa33;
	color: #ffffff;
}

QPushButton:pressed {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #cc6600, stop:1 #aa5500);
	border: 2px solid #aa5500;
	color: #ffffff;
	padding-top: 11px;
	padding-bottom: 9px;
}

QPushButton:checked {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #ffaa33, stop:1 #ff8800);
	border: 2px solid #ffaa33;
	color: #ffffff;
}

QPushButton:disabled {
	background: #3e3e42;
	border: 2px solid #2d2d30;
	color: #6e6e73;
}

QPushButton:focus {
	border: 2px solid #ffaa33;
	outline: none;
}


/* ===== QToolBar ===== */
QToolBar {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #252526, stop:1 #1e1e1e);
    border-right: 2px solid #2d2d30;
	border-radius: 0px;
}

QToolButton {
	background-color: transparent;
	margin: 5px 5px;
    width: 48px;
    height: 48px;
    border-radius: 10px;
    border: 2px solid transparent;
}

QToolButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    border: 2px solid #ff8800;
}

QToolButton:checked {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    border: 2px solid #ffaa33;
}


/* ===== QLCDNumber ===== */
QLCDNumber {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #1e1e1e, stop:1 #252526);
    border: 2px solid #ff8800;
    border-radius: 12px;
    color: #00ff00;
    padding: 4px;
}


/* ===== QSlider ===== */
QSlider:horizontal {
    min-height: 32px;
}

QSlider::groove:horizontal {
	background-color: #2d2d30;
	height: 6px;
    border-radius: 3px;
    border: 1px solid #3e3e42;
}

QSlider::sub-page:horizontal {
	background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                              stop:0 #ff8800, stop:1 #ffaa33);
    border-radius: 3px;
}

QSlider::add-page:horizontal {
	background-color: #3e3e42;
    border-radius: 3px;
}

QSlider::handle:horizontal {
	width: 22px;
    margin-top: -9px;
    margin-bottom: -9px;
    border-radius: 11px;
	background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #e0e0e0, stop:1 #b0b0b0);
    border: 2px solid #ff8800;
}

QSlider::handle:horizontal:hover {
	background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #ffffff, stop:1 #e0e0e0);
    border: 2px solid #ffaa33;
}

QSlider::sub-page:horizontal:disabled {
	background-color: #3e3e42;
	border-color: #2d2d30;
}

QSlider::add-page:horizontal:disabled {
	background-color: #252526;
	border-color: #1e1e1e;
}

QSlider::handle:horizontal:disabled {
	background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(62, 62, 66, 255),
                              stop:0.7 rgba(45, 45, 48, 100));
	border: 2px solid #3e3e42;
}

QSlider:vertical {
    min-width: 32px;
}

QSlider::groove:vertical {
    width: 6px;
    background-color: #2d2d30;
    border-radius: 3px;
    border: 1px solid #3e3e42;
}

QSlider::handle:vertical {
    height: 22px;
    margin-left: -9px;
    margin-right: -9px;
    border-radius: 11px;
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #e0e0e0, stop:1 #b0b0b0);
    border: 2px solid #ff8800;
}

QSlider::handle:vertical:hover {
    background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0 #ffffff, stop:0.7 #ffffff, stop:1 #e0e0e0);
    border: 2px solid #ffaa33;
}

QSlider::sub-page:vertical {
	background-color: #3e3e42;
    border-radius: 3px;
}

QSlider::add-page:vertical {
	background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                              stop:0 #ff8800, stop:1 #ffaa33);
    border-radius: 3px;
}

QSlider::sub-page:vertical:disabled {
	background-color: #252526;
	border-color: #1e1e1e;
}

QSlider::add-page:vertical:disabled {
	background-color: #3e3e42;
	border-color: #2d2d30;
}

QSlider::handle:vertical:disabled {
	background: qradialgradient(spread:reflect, cx:0.5, cy:0.5, radius:0.5,
                              fx:0.5, fy:0.5,
                              stop:0.6 rgba(62, 62, 66, 255),
                              stop:0.7 rgba(45, 45, 48, 100));
	border: 2px solid #3e3e42;
}





/* ===== QTabWidget and QTabBar ===== */
QTabWidget {
    background-color: transparent;
}

QTabWidget::pane {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
    border: 2px solid #ff8800;
    border-radius: 12px;
    top: -1px;
    padding: 8px;
}

QTabWidget::tab-bar {
    alignment: left;
}

QTabBar {
    background-color: transparent;
}

QTabBar::tab {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    color: #b0b0b0;
    border: px solid #3e3e42;
    border-bottom: none;
    border-top-left-radius: 10px;
    border-top-right-radius: 10px;
    padding: 10px 24px;
    margin-right: 4px;
    font-size: 13px;
    font-weight: 500;
}

QTabBar::tab:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    color: #ffffff;
    border: 2px solid #ff8800;
    border-bottom: none;
    font-weight: bold;
    padding-bottom: 12px;
}

QTabBar::tab:!selected {
    margin-top: 4px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #252526, stop:1 #1e1e1e);
}

QTabBar::tab:!selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    color: #e0e0e0;
    border: 2px solid #ff8800;
}

QTabBar::tab:first:!selected {
    margin-left: 0px;
}

QTabBar::tab:last {
    margin-right: 0;
}

QTabBar::tab:only-one {
    margin: 0;
}

QTabBar::tab:disabled {
    background-color: #252526;
    color: #6e6e73;
    border: 2px solid #3e3e42;
}

/* Vertical tabs (left/right) */
QTabBar::tab:left,
QTabBar::tab:right {
    border-top-left-radius: 0px;
    border-top-right-radius: 0px;
}

QTabBar::tab:left {
    border-left: 2px solid #ff8800;
    border-right: none;
    border-top-left-radius: 10px;
    border-bottom-left-radius: 10px;
    margin-bottom: 4px;
    margin-right: 0px;
}

QTabBar::tab:right {
    border-right: 2px solid #ff8800;
    border-left: none;
    border-top-right-radius: 10px;
    border-bottom-right-radius: 10px;
    margin-bottom: 4px;
    margin-left: 0px;
}

QTabBar::tab:left:selected,
QTabBar::tab:right:selected {
    padding-bottom: 10px;
}

/* Close button on tabs */
QTabBar::close-button {
    image: url(:/images/icons/cross.png);
    subcontrol-position: right;
    border-radius: 6px;
    background-color: transparent;
    padding: 2px;
}

QTabBar::close-button:hover {
    background-color: #e81123;
}

QTabBar::close-button:pressed {
    background-color: #c50f1f;
}

/* ===== QToolTip ===== */
QToolTip {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    background-clip: border;
    color: #ffffff;
    border: 2px solid #ffaa33;
    border-radius: 8px;
    padding: 8px 14px;
    font-size: 13px;
    font-weight: 500;
    margin: 2px;
}


/* ===== QScrollBar ===== */
QScrollBar:vertical {
    border: none;
    background: #1e1e1e;
    width: 18px;
    border-radius: 9px;
}

QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff8800, stop:1 #cc6600);
    min-height: 30px;
    border-radius: 7px;
    margin: 2px;
}

QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
    background: none;
}

QScrollBar:horizontal {
    border: none;
    background: #1e1e1e;
    height: 18px;
    border-radius: 9px;
}

QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    min-width: 30px;
    border-radius: 7px;
    margin: 2px;
}

QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QScrollBar::sub-line:horizontal {
	border: none;
	background: none;
}

QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QComboBox ===== */
QComboBox {
    background: transparent;
    combobox-popup: 0;
    border: 2px solid #ff8800;
    border-radius: 10px;
    padding: 8px 12px;
    min-width: 6em;
    color: #e0e0e0;
    font-size: 13px;
    selection-background-color: #ff8800;
    selection-color: #ffffff;
}

QComboBox:hover {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QComboBox:focus {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QComboBox:on {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    padding-top: 9px;
    padding-bottom: 7px;
}

QComboBox:disabled {
    background-color: #252526;
    border: 2px solid #3e3e42;
    color: #6e6e73;
}

QComboBox:editable {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
    color: #e0e0e0;
}

QComboBox:editable:focus {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QComboBox::drop-down {
    subcontrol-origin: padding;
    subcontrol-position: center right;
    width: 32px;
    border-left: 2px solid #ff8800;
    border-top-right-radius: 8px;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
}

QComboBox::drop-down:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QComboBox::drop-down:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QComboBox::drop-down:disabled {
    background-color: #3e3e42;
    border-left: 2px solid #3e3e42;
}

QComboBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 14px;
    height: 14px;
}

QComboBox::down-arrow:disabled {
    image: none;
}

QComboBox::down-arrow:on {
    top: 1px;
}

QComboBox QAbstractItemView {
    background-color: #2a2a2d;
    border: 2px solid #ff8800;
    border-radius: 8px;
    padding: 4px;
    margin: 0px;
    selection-background-color: #ff8800;
    selection-color: #ffffff;
    outline: none;
}

QComboBox QAbstractItemView::item {
    padding: 8px 12px;
    margin: 2px;
    border: none;
    border-radius: 6px;
    color: #e0e0e0;
    min-height: 24px;
}

QComboBox QAbstractItemView::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff8800, stop:1 #ffaa33);
    color: #ffffff;
    border-left: 4px solid #ffaa33;
    font-weight: bold;
}

QComboBox QAbstractItemView::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    color: #ffffff;
}

QComboBox QAbstractItemView::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ffaa33, stop:1 #aa5500);
    color: #ffffff;
    border-left: 4px solid #aa5500;
    font-weight: bold;
}

/* ===== QCheckBox ===== */
QCheckBox {
    spacing: 12px;
    font-size: 13px;
    color: #e0e0e0;
    padding: 4px;
}

QCheckBox:hover {
    color: #ffffff;
}

QCheckBox:disabled {
    color: #6e6e73;
}

QCheckBox::indicator {
    width: 20px;
    height: 20px;
    border-radius: 6px;
    border: 2px solid #ff8800;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
}

QCheckBox::indicator:hover {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QCheckBox::indicator:pressed {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d2d30, stop:1 #252526);
}

QCheckBox::indicator:checked {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:checked:hover {
    border: 2px solid #cc6600;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QCheckBox::indicator:checked:pressed {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QCheckBox::indicator:unchecked:disabled {
    border: 2px solid #3e3e42;
    background-color: #252526;
}

QCheckBox::indicator:checked:disabled {
    border: 2px solid #3e3e42;
    background-color: #3e3e42;
    image: url(:/images/icons/Icons-checkmark-16.png);
}

QCheckBox::indicator:indeterminate {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QCheckBox::indicator:indeterminate:hover {
    border: 2px solid #aa5500;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}



/* ===== QRadioButton ===== */
QRadioButton {
    spacing: 12px;
    font-size: 13px;
    color: #e0e0e0;
    padding: 4px;
}

QRadioButton:hover {
    color: #ffffff;
}

QRadioButton:disabled {
    color: #6e6e73;
}

QRadioButton::indicator {
    width: 20px;
    height: 20px;
    border-radius: 10px;
    border: 2px solid #ff8800;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
}

QRadioButton::indicator:hover {
    border: 2px solid #ffaa33;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #3e3e42,
                                stop:0.7 #2d2d30,
                                stop:1 #252526);
}

QRadioButton::indicator:pressed {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d2d30, stop:1 #252526);
}

QRadioButton::indicator:checked {
    border: 2px solid #ffaa33;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #ffaa33,
                                stop:0.3 #ff8800,
                                stop:0.6 #cc6600,
                                stop:1 #aa5500);
}

QRadioButton::indicator:checked:hover {
    border: 2px solid #cc6600;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #ffbb00,
                                stop:0.3 #ffaa33,
                                stop:0.6 #ff8800,
                                stop:1 #cc6600);
}

QRadioButton::indicator:checked:pressed {
    border: 2px solid #ffaa33;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #ff8800,
                                stop:0.5 #cc6600,
                                stop:1 #aa5500);
}

QRadioButton::indicator:unchecked:disabled {
    border: 2px solid #3e3e42;
    background-color: #252526;
}

QRadioButton::indicator:checked:disabled {
    border: 2px solid #3e3e42;
    background: qradialgradient(spread:pad, cx:0.5, cy:0.5, radius:0.5,
                                fx:0.5, fy:0.5,
                                stop:0 #3e3e42,
                                stop:0.5 #2d2d30,
                                stop:1 #252526);
}

/* ===== QLineEdit ===== */
QLineEdit {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
    border: 2px solid #ff8800;
    border-radius: 10px;
    padding: 8px 12px;
    color: #ff8800;
    selection-background-color: #ff8800;
    selection-color: #ffffff;
    font-size: 13px;
}

QLineEdit:hover {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QLineEdit:focus {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
    outline: none;
}

QLineEdit:disabled {
    background-color: #252526;
    border: 2px solid #3e3e42;
    color: #6e6e73;
}

QLineEdit[readOnly="true"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d2d30, stop:1 #252526);
    border: 2px solid #3e3e42;
    color: #b0b0b0;
}

/* ===== QSpinBox ===== */
QSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
    border: 2px solid #ff8800;
    border-radius: 10px;
    padding: 8px;
    color: #e0e0e0;
    selection-background-color: #ff8800;
    selection-color: #ffffff;
    font-size: 13px;
}

QSpinBox:hover {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QSpinBox:focus {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QSpinBox:disabled {
    background-color: #252526;
    border: 2px solid #3e3e42;
    color: #6e6e73;
}

QSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #ff8800;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
}

QSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QSpinBox::up-button:disabled {
    background-color: #3e3e42;
}

QSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::up-arrow:disabled {
    image: none;
}

QSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #ff8800;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
}

QSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QSpinBox::down-button:disabled {
    background-color: #3e3e42;
}

QSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QDoubleSpinBox ===== */
QDoubleSpinBox {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3a3a3d, stop:1 #2d2d30);
    border: 2px solid #ff8800;
    border-radius: 10px;
    padding: 8px;
    color: #e0e0e0;
    selection-background-color: #ff8800;
    selection-color: #ffffff;
    font-size: 13px;
}

QDoubleSpinBox:hover {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QDoubleSpinBox:focus {
    border: 2px solid #ffaa33;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #2d2d30);
}

QDoubleSpinBox:disabled {
    background-color: #252526;
    border: 2px solid #3e3e42;
    color: #6e6e73;
}

QDoubleSpinBox::up-button {
    subcontrol-origin: border;
    subcontrol-position: top right;
    width: 22px;
    border-left: 2px solid #ff8800;
    border-top-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
}

QDoubleSpinBox::up-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QDoubleSpinBox::up-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QDoubleSpinBox::up-button:disabled {
    background-color: #3e3e42;
}

QDoubleSpinBox::up-arrow {
    image: url(:/images/icons/Icons-dropup-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::up-arrow:disabled {
    image: none;
}

QDoubleSpinBox::down-button {
    subcontrol-origin: border;
    subcontrol-position: bottom right;
    width: 22px;
    border-left: 2px solid #ff8800;
    border-bottom-right-radius: 8px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
}

QDoubleSpinBox::down-button:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QDoubleSpinBox::down-button:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QDoubleSpinBox::down-button:disabled {
    background-color: #3e3e42;
}

QDoubleSpinBox::down-arrow {
    image: url(:/images/icons/Icons-dropdown-arrow-16.png);
    width: 12px;
    height: 12px;
}

QDoubleSpinBox::down-arrow:disabled {
    image: none;
}

/* ===== QLabel ===== */
QLabel {
    background-color: transparent;
    color: #e0e0e0;
    padding: 6px;
    font-size: 14px;
    font-weight: 500;
    border: 0px;
    border-radius: 6px;
}

QLabel[frameShape="6"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #808080, stop:1 #707070);
    border: 2px solid #3c6844;
    border-radius: 8px;
    padding: 8px 12px;
    color: #e0e0e0;
    font-weight: 600;
}

QLabel[frameShape="1"] {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #3c6844, stop:1 #4d8a5a);
    border: 2px solid #3c6844;
    border-radius: 10px;
    padding: 10px 16px;
    color: #ffffff;
    font-weight: bold;
    letter-spacing: 0.5px;
}

QLabel:disabled {
    color: #a0a0a0;
    background-color: #686868;
}

/* ===== QTableWidget ===== */
QTableWidget {
    background-color: #2d2d30;
    border: 2px solid #ff8800;
    border-radius: 12px;
    gridline-color: #3e3e42;
    selection-background-color: transparent;
    selection-color: #ffffff;
    font-size: 13px;
    outline: none;
    padding: 5px;
}

QTableWidget::item {
    padding: 8px;
    border: none;
    background-color: #2d2d30;
    border-radius: 12px;
    color: #e0e0e0;
}

QTableWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff8800, stop:1 #cc6600);
    color: #ffffff;
    border-radius: 12px;
    font-weight: bold;
}

QTableWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3e3e42, stop:1 #353538);
    color: #ffaa33;
    border-radius: 12px;
}

QTableWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ffaa33, stop:1 #ff8800);
    color: #ffffff;
    border-radius: 12px;
}

QHeaderView::section {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    color: #ffffff;
    padding: 8px 12px;
    border: 1px solid #aa5500;
    border-radius: 6px;
    font-weight: bold;
    font-size: 13px;
    text-align: center;
}

QHeaderView::section:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QHeaderView::section:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QTableCornerButton::section {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    border: 1px solid #aa5500;
    border-radius: 6px;
}

QTableWidget QScrollBar:vertical {
    border: none;
    background: #1e1e1e;
    width: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:vertical {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ff8800, stop:1 #cc6600);
    min-height: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:vertical:hover {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QTableWidget QScrollBar::handle:vertical:pressed {
    background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QTableWidget QScrollBar::add-line:vertical, QTableWidget QScrollBar::sub-line:vertical {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::add-page:vertical, QTableWidget QScrollBar::sub-page:vertical {
    background: none;
}

QTableWidget QScrollBar:horizontal {
    border: none;
    background: #1e1e1e;
    height: 18px;
    margin: 5px 5px 5px 5px;
    border-radius: 9px;
}

QTableWidget QScrollBar::handle:horizontal {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ff8800, stop:1 #cc6600);
    min-width: 30px;
    border-radius: 9px;
    margin: 2px;
}

QTableWidget QScrollBar::handle:horizontal:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #ffaa33, stop:1 #ff8800);
}

QTableWidget QScrollBar::handle:horizontal:pressed {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #cc6600, stop:1 #aa5500);
}

QTableWidget QScrollBar::add-line:horizontal {
    border: none;
    background: none;
    height: 0px;
}

QTableWidget QScrollBar::sub-line:horizontal {
    border: none;
    background: none;
}

QTableWidget QScrollBar::left-arrow:horizontal, QTableWidget QScrollBar::right-arrow:horizontal {
    border: none;
    width: 0px;
    height: 0px;
    background: none;
}

QTableWidget QScrollBar::add-page:horizontal, QTableWidget QScrollBar::sub-page:horizontal {
    background: none;
}

/* ===== QListWidget ===== */
QListWidget {
    background-color: rgb(58, 59, 58);
    border: 2px solid qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                      stop:0 #404040, stop:1 #505050);
    border-radius: 12px;
    padding: 2px;
    color: #e8e8e8;
    font-size: 12px;
    outline: none;
}

QListWidget::item {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #2d2d2d, stop:1 #353535);
    border: 1px solid #454545;
    border-radius: 8px;
    padding: 6px 12px;
    margin: 5px 3px;
    color: #d5d5d5;
    font-weight: 500;
}

QListWidget::item:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3f3f3f, stop:1 #505050);
    border: 1px solid #ff8800;
    color: #ffffff;
    padding-left: 12px;
}

QListWidget::item:selected {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3f3f3f, stop:1 #474747);
    border: 1px solid #ff8800;
    color: #ffffff;
    padding-left: 12px;
}
QListWidget::item:selected:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                                stop:0 #3f3f3f, stop:1 #505050);
    border: 1px solid #ff8800;
    color: #ffffff;
    padding-left: 22px;
}

#CentralMainWindowHomeWidget {
    background-color: rgba(#fff, 0.5);
}
    )";
}

}
