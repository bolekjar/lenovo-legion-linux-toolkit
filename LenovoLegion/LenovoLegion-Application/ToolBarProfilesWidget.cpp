#include "ToolBarProfilesWidget.h"
#include "Core/LoggerHolder.h"
#include "ui_ToolBarProfilesWidget.h"

#include "Utils.h"
#include "WidgetMessage.h"

// Include daemon data provider constants
#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"
#include "../LenovoLegion-PrepareBuild/Profiles.pb.h"

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

    try {
        showProfiles(m_dataProvider->getDataMessage<legion::messages::Profiles>(legion::messages::DataType::PROFILES));
    }
    catch (const std::exception& e) {
        LOG_E(QString("Failed to read profiles: ").append(e.what()));
        showProfiles({});
    }
}

void ToolBarProfilesWidget::showProfiles(const legion::messages::Profiles &profiles)
{
    ui->listWidget_Profiles->clear();
    m_profiles.clear();

    for (const auto& profile : profiles.profiles()) {
        const QString name = QString::fromStdString(std::string(profile.name()));

        m_profiles[name] = QString::fromStdString(std::string(profile.description()));
        ui->listWidget_Profiles->addItem(name);
    }

    LOG_T(QString("Loaded %1 profiles").arg(m_profiles.size()));
}

void ToolBarProfilesWidget::profileCommand(legion::messages::ProfileCommand::Action action, const QString &name, const QString &description)
{
    legion::messages::ProfileCommand command;

    command.set_action(action);
    command.set_name(name.toStdString());
    command.set_description(description.toStdString());

    const QByteArray response = m_dataProvider->setDataMessage(legion::messages::DataType::PROFILES, command);

    legion::messages::Profiles profiles;

    if (!profiles.ParseFromArray(response.constData(), static_cast<int>(response.size()))) {
        THROW_EXCEPTION(DataProvider::exception_T, DataProvider::ERROR_CODES::PARSER_ERROR, "Parse error");
    }

    showProfiles(profiles);
}

void ToolBarProfilesWidget::updateProfileDetails(const QString& profileName)
{
    LOG_T(QString("Updating profile details for: ").append(profileName));
    
    ui->lineEdit_ProfileName->setText(profileName);
    
    const auto it = m_profiles.find(profileName);
    ui->lineEdit_ProfileDescription->setText(it != m_profiles.end() ? it->second : QString());
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
    if (m_profiles.count(profileName) > 0) {
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
        /*
         * Daemon saves the settable part of the current hardware state
         */
        profileCommand(legion::messages::ProfileCommand::SAVE, profileName, profileDescription);

        LOG_T(QString("Profile saved successfully: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap infoIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(infoIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Saved");
        msgBox.setText(QString("Profile '").append(profileName).append("' has been saved successfully."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
    catch (const std::exception& e) {
        LOG_E(QString("Failed to save profile: ").append(e.what()));
        
        QMessageBox msgBox(this);
        QPixmap errorIcon(":/images/icons/cross.png");
        msgBox.setIconPixmap(errorIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Save Failed");
        msgBox.setText(QString("Failed to save profile: ").append(e.what()));
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
        /*
         * Daemon applies the profile, values which can not be set are skipped and logged by the daemon.
         * Other pages read the data again when they are opened.
         */
        profileCommand(legion::messages::ProfileCommand::LOAD, profileName);

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
        msgBox.setText(QString("Failed to load profile: ").append(e.what()));
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
    
    try {
        profileCommand(legion::messages::ProfileCommand::DELETE, profileName);

        LOG_T(QString("Profile deleted successfully: ").append(profileName));
        
        QMessageBox msgBox(this);
        QPixmap infoIcon(":/images/icons/info.png");
        msgBox.setIconPixmap(infoIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Profile Deleted");
        msgBox.setText(QString("Profile '").append(profileName).append("' has been deleted."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        
        ui->lineEdit_ProfileName->clear();
        ui->lineEdit_ProfileDescription->clear();
    }
    catch (const std::exception& e) {
        LOG_E(QString("Failed to delete profile: ").append(profileName).append(" : ").append(e.what()));
        
        QMessageBox msgBox(this);
        QPixmap errorIcon(":/images/icons/cross.png");
        msgBox.setIconPixmap(errorIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        msgBox.setWindowTitle("Delete Failed");
        msgBox.setText(QString("Failed to delete profile: ").append(e.what()));
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
