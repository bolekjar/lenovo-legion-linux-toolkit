#include "ToolBarProfilesWidget.h"
#include "Core/LoggerHolder.h"
#include "ui_ToolBarProfilesWidget.h"

#include "Utils.h"
#include "WidgetMessage.h"
#include "Settings.h"

// Include daemon data provider constants
#include "../LenovoLegion-Daemon/SysFsDataProviderPowerProfile.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUOptions.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUFrequency.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderFanCurve.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderFanOption.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUSMT.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUPower.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderGPUPower.h"
#include "../LenovoLegion-Daemon/DataProviderNvidiaNvml.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderIntelMSR.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderOther.h"

#include <QMessageBox>


namespace LenovoLegionGui {


ToolBarProfilesWidget::ToolBarProfilesWidget(DataProvider* dataProvider,QWidget *parent)
    : ToolBarWidget(dataProvider,parent)
    , ui(new Ui::ToolBarProfilesWidget)
{

    ui->setupUi(this);
    
    // Connect signals
    connectSignals();
    
    // Load profiles list
    loadProfilesList();

    /*
     * Initialize actions map
     */

    m_defaultActionsMap["add"].push_back([]() {
        // Profiles initialization
        LOG_D("ToolBarProfilesWidget initialized");
    });


    m_defaultActionsMap["remove"].push_back([]() {
        // Profiles cleanup if needed
    });

    m_defaultActionsMap["refresh"].push_back([]() {
        // Refresh profiles list if needed
    });

    Utils::Task::insertTasksBack(m_asyncTasks,m_defaultActionsMap["add"]);
}

void ToolBarProfilesWidget::dataProviderEvent(const legion::messages::Notification &event)
{
    // Profiles widget doesn't need to react to most notifications
    Q_UNUSED(event);
}

void ToolBarProfilesWidget::cleanup()
{
    ToolBarWidget::cleanup();
}

ToolBarProfilesWidget::~ToolBarProfilesWidget()
{
    delete ui;
}

void ToolBarProfilesWidget::widgetEvent(const WidgetMessage &event)
{
    // Handle widget events if needed
    Q_UNUSED(event);
}

void ToolBarProfilesWidget::connectSignals()
{
    // Connect button signals
    connect(ui->pushButton_SaveProfile, &QPushButton::clicked, this, &ToolBarProfilesWidget::onSaveProfile);
    connect(ui->pushButton_LoadProfile, &QPushButton::clicked, this, &ToolBarProfilesWidget::onLoadProfile);
    connect(ui->pushButton_DeleteProfile, &QPushButton::clicked, this, &ToolBarProfilesWidget::onDeleteProfile);
    
    // Connect list widget selection signal
    connect(ui->listWidget_Profiles, &QListWidget::itemSelectionChanged, this, &ToolBarProfilesWidget::onProfileSelected);
}

void ToolBarProfilesWidget::loadProfilesList()
{
    LOG_T("Loading profiles list");
    
    ui->listWidget_Profiles->clear();
    
    QStringList profiles = ProfileManager::listProfiles();
    for (const QString& profileName : profiles) {
        ui->listWidget_Profiles->addItem(profileName);
    }
    
    LOG_T(QString("Loaded %1 profiles").arg(profiles.size()));
}

void ToolBarProfilesWidget::updateProfileDetails(const QString& profileName)
{
    LOG_T(QString("Updating profile details for: ").append(profileName));
    
    ui->lineEdit_ProfileName->setText(profileName);
    
    QString description = ProfileManager::getProfileDescription(profileName);
    ui->lineEdit_ProfileDescription->setText(description);
}

void ToolBarProfilesWidget::onSaveProfile()
{
    LOG_T("Save Profile button clicked");
    
    QString profileName = ui->lineEdit_ProfileName->text().trimmed();
    QString profileDescription = ui->lineEdit_ProfileDescription->text().trimmed();
    
    if (profileName.isEmpty()) {
        LOG_W("Profile name is empty");
        
        QMessageBox msgBox(this);
        QPixmap warningIcon(":/images/icons/warning.png");
        msgBox.setIconPixmap(warningIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Invalid Profile Name");
        msgBox.setText("Please enter a profile name.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    // Check if profile already exists
    if (ProfileManager::profileExists(profileName)) {
        QMessageBox msgBox(this);
        QPixmap dialogIcon(":/images/icons/dialog.png");
        msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Exists");
        msgBox.setText(QString("Profile '").append(profileName).append("' already exists. Overwrite it?"));
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        
        if (msgBox.exec() != QMessageBox::Yes) {
            return;
        }
    }
    
    LOG_D(QString("Saving profile: ").append(profileName));
    
    try {
        // Create profile settings object
        ProfileSettings profile(profileName);
        
        // Save description
        profile.saveDescription(profileDescription);
        
        // Read current settings from daemon and save to profile
        auto powerProfile = m_dataProvider->getDataMessage<legion::messages::PowerProfile>(
            LenovoLegionDaemon::SysFsDataProviderPowerProfile::dataType);
        profile.savePowerProfile(powerProfile);
        
        auto cpuOptions = m_dataProvider->getDataMessage<legion::messages::CPUOptions>(
            LenovoLegionDaemon::SysFsDataProviderCPUOptions::dataType);
        profile.saveCPUOptions(cpuOptions);
        
        auto cpuFrequency = m_dataProvider->getDataMessage<legion::messages::CPUFrequency>(
            LenovoLegionDaemon::SysFsDataProviderCPUFrequency::dataType);
        profile.saveCPUFrequency(cpuFrequency);
        
        // Check if power profile is CUSTOM - only save custom settings if it is
        bool isCustomProfile = powerProfile.thermal_mode() &&
                               powerProfile.thermal_mode() == legion::messages::PowerProfile_Profiles_POWER_PROFILE_CUSTOM;
        
        if (isCustomProfile) {
            LOG_T("Power profile is CUSTOM - saving FanCurve, CPUPower, and GPUPower");
            
            auto fanCurve = m_dataProvider->getDataMessage<legion::messages::FanCurve>(
                LenovoLegionDaemon::SysFsDataProviderFanCurve::dataType);
            profile.saveFanCurve(fanCurve);
            
            auto cpuPower = m_dataProvider->getDataMessage<legion::messages::CPUPower>(
                LenovoLegionDaemon::SysFsDataProviderCPUPower::dataType);
            profile.saveCPUPower(cpuPower);
            
            auto gpuPower = m_dataProvider->getDataMessage<legion::messages::GPUPower>(
                LenovoLegionDaemon::SysFsDataProviderGPUPower::dataType);
            profile.saveGPUPower(gpuPower);
        } else {
            LOG_T("Power profile is not CUSTOM - skipping FanCurve, CPUPower, and GPUPower");
        }
        
        auto fanOption = m_dataProvider->getDataMessage<legion::messages::FanOption>(
            LenovoLegionDaemon::SysFsDataProviderFanOption::dataType);
        profile.saveFanOption(fanOption);
        
        auto cpuSmt = m_dataProvider->getDataMessage<legion::messages::CPUSMT>(
            LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType);
        profile.saveCPUSMT(cpuSmt);
        
        auto nvidiaNvml = m_dataProvider->getDataMessage<legion::messages::NvidiaNvml>(
            LenovoLegionDaemon::DataProviderNvidiaNvml::dataType);
        profile.saveNvidiaNvml(nvidiaNvml);
        
        auto intelMSR = m_dataProvider->getDataMessage<legion::messages::CpuIntelMSR>(
            LenovoLegionDaemon::SysFsDataProviderIntelMSR::dataType);

        intelMSR.mutable_analogio()->set_offset(((intelMSR.analogio().offset() > 0 ? intelMSR.analogio().offset() + 999 : intelMSR.analogio().offset() - 999 ) / 1000) * 1000);
        intelMSR.mutable_cache()->set_offset(((intelMSR.cache().offset() > 0 ? intelMSR.cache().offset() + 999 : intelMSR.cache().offset() - 999 ) / 1000) * 1000);
        intelMSR.mutable_cpu()->set_offset(((intelMSR.cpu().offset() > 0 ? intelMSR.cpu().offset() + 999 : intelMSR.cpu().offset() - 999) / 1000) * 1000);
        intelMSR.mutable_gpu()->set_offset(((intelMSR.gpu().offset() > 0 ? intelMSR.gpu().offset() + 999 : intelMSR.gpu().offset() - 999 ) / 1000) * 1000);
        intelMSR.mutable_uncore()->set_offset(((intelMSR.uncore().offset() > 0 ? intelMSR.uncore().offset() + 999 : intelMSR.uncore().offset() - 999 ) / 1000) * 1000);

        profile.saveIntelMSR(intelMSR);
        
        auto otherSettings = m_dataProvider->getDataMessage<legion::messages::OtherSettings>(
            LenovoLegionDaemon::SysFsDataProviderOther::dataType);
        profile.saveOther(otherSettings);
        
        LOG_T(QString("Profile saved successfully: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap infoIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(infoIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Saved");
        msgBox.setText(QString("Profile '").append(profileName).append("' has been saved successfully."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        
        loadProfilesList();
    }
    catch (const std::exception& e) {
        LOG_E(QString("Failed to save profile: ").append(e.what()));
        
        QMessageBox msgBox(this);
        QPixmap errorIcon(":/images/icons/cross.png");
        msgBox.setIconPixmap(errorIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Save Failed");
        msgBox.setText("Failed to save profile. Check logs for details.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void ToolBarProfilesWidget::onLoadProfile()
{
    LOG_T("Load Profile button clicked");
    
    QListWidgetItem* selectedItem = ui->listWidget_Profiles->currentItem();
    if (!selectedItem) {
        LOG_W("No profile selected");
        
        QMessageBox msgBox(this);
        QPixmap warningIcon(":/images/icons/warning.png");
        msgBox.setIconPixmap(warningIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select a profile to load.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    QString profileName = selectedItem->text();
    
    QMessageBox msgBox(this);
    QPixmap dialogIcon(":/images/icons/dialog.png");
    msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setWindowTitle("Load Profile");
    msgBox.setText(QString("Load profile '").append(profileName).append("'? This will apply all saved settings to the daemon."));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    LOG_T(QString("Loading profile: ").append(profileName));
    
    try {
        ProfileSettings profile(profileName);
        
        // Load settings from profile
        legion::messages::PowerProfile powerProfile;
        profile.loadPowerProfile(powerProfile);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderPowerProfile::dataType, powerProfile);
        
        legion::messages::CPUOptions cpuOptions;
        profile.loadCPUOptions(cpuOptions);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUOptions::dataType, cpuOptions);
        
        legion::messages::CPUFrequency cpuFrequency;
        profile.loadCPUFrequency(cpuFrequency);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUFrequency::dataType, cpuFrequency);
        
        legion::messages::FanCurve fanCurve;
        profile.loadFanCurve(fanCurve);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderFanCurve::dataType, fanCurve);
        
        legion::messages::FanOption fanOption;
        profile.loadFanOption(fanOption);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderFanOption::dataType, fanOption);
        
        legion::messages::CPUSMT cpuSmt;
        profile.loadCPUSMT(cpuSmt);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType, cpuSmt);
              
        legion::messages::CPUPower cpuPower;
        profile.loadCPUPower(cpuPower);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUPower::dataType, cpuPower);
        
        legion::messages::GPUPower gpuPower;
        profile.loadGPUPower(gpuPower);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderGPUPower::dataType, gpuPower);
        
        legion::messages::NvidiaNvml nvidiaNvml;
        profile.loadNvidiaNvml(nvidiaNvml);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::DataProviderNvidiaNvml::dataType, nvidiaNvml);
        
        legion::messages::CpuIntelMSR intelMSR;
        profile.loadIntelMSR(intelMSR);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderIntelMSR::dataType, intelMSR);
        
        legion::messages::OtherSettings otherSettings;
        profile.loadOther(otherSettings);
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderOther::dataType, otherSettings);
        
        LOG_T(QString("Profile loaded successfully: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap infoIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(infoIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Loaded");
        msgBox.setText(QString("Profile '").append(profileName).append("' has been loaded and applied."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    catch (const std::exception& e) {
        LOG_E(QString("Failed to load profile: ").append(e.what()));
        
        QMessageBox msgBox(this);
        QPixmap errorIcon(":/images/icons/cross.png");
        msgBox.setIconPixmap(errorIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Load Failed");
        msgBox.setText("Failed to load profile. Check logs for details.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void ToolBarProfilesWidget::onDeleteProfile()
{
    LOG_T("Delete Profile button clicked");
    
    QListWidgetItem* selectedItem = ui->listWidget_Profiles->currentItem();
    if (!selectedItem) {
        LOG_W("No profile selected");
        
        QMessageBox msgBox(this);
        QPixmap warningIcon(":/images/icons/warning.png");
        msgBox.setIconPixmap(warningIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("No Selection");
        msgBox.setText("Please select a profile to delete.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    QString profileName = selectedItem->text();
    
    QMessageBox msgBox(this);
    QPixmap dialogIcon(":/images/icons/dialog.png");
    msgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    msgBox.setWindowTitle("Delete Profile");
    msgBox.setText(QString("Are you sure you want to delete profile '").append(profileName).append("'?"));
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    if (msgBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    LOG_T(QString("Deleting profile: ").append(profileName));
    
    if (ProfileManager::deleteProfile(profileName)) {
        LOG_T(QString("Profile deleted successfully: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap infoIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(infoIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Deleted");
        msgBox.setText(QString("Profile '").append(profileName).append("' has been deleted."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        
        loadProfilesList();
        ui->lineEdit_ProfileName->clear();
        ui->lineEdit_ProfileDescription->clear();
    } else {
        LOG_E(QString("Failed to delete profile: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap errorIcon(":/images/icons/cross.png");
        msgBox.setIconPixmap(errorIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Delete Failed");
        msgBox.setText("Failed to delete profile.");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void ToolBarProfilesWidget::onProfileSelected()
{
    QListWidgetItem* selectedItem = ui->listWidget_Profiles->currentItem();
    if (!selectedItem) {
        LOG_T("No profile selected");
        ui->lineEdit_ProfileName->clear();
        ui->lineEdit_ProfileDescription->clear();
        return;
    }
    
    QString profileName = selectedItem->text();
    LOG_T(QString("Profile selected: ").append(profileName));
    
    updateProfileDetails(profileName);
}


}
