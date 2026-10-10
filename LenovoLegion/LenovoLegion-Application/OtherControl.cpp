// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "OtherControl.h"
#include "ui_OtherControl.h"

#include "DataProvider.h"
#include "MainWindow.h"

#include "../LenovoLegion-Daemon/SysFsDataProviders/SysFsDataProviderOther.h"
#include "../LenovoLegion-Daemon/SysFsDataProviders/SysFsDataProviderOtherGpuSwitch.h"
#include "../LenovoLegion-Daemon/SysFsDataProviders/SysFsDataProviderCPUPState.h"

#include <Core/LoggerHolder.h>

#include <QMessageBox>

namespace LenovoLegionGui {

const QString OtherControl::NAME = "Other Settings";


const QMap<QString, legion::messages::CPUPState> OtherControl::CPU_PSTATE_PRESET = {
    {"Default",[](){
            legion::messages::CPUPState pstate;

            pstate.set_hwp_dynamic_boost(0);
            pstate.set_no_turbo(0);
            pstate.set_max_perf_pct(100);
            pstate.set_min_perf_pct(14);
            pstate.set_status("active");


            return pstate;
        }()}
};

OtherControl::OtherControl(DataProvider *dataProvider, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtherControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    /*
     * Refresh data
     */
    refresh();
}

OtherControl::~OtherControl()
{
    delete ui;
}

void OtherControl::refresh()
{
    readData();
    renderData();
    renderGpuSwitchData();
    renderCPUPstate();
}

void OtherControl::renderData()
{
    // Block signals to prevent triggering applySettings during refresh
    ui->checkBox_DisableTouchpad->blockSignals(true);
    ui->checkBox_DisableWinKey->blockSignals(true);

    ui->checkBox_DisableTouchpad->setVisible(m_otherSettingsData.touch_pad().supported());
    ui->checkBox_DisableWinKey->setVisible(m_otherSettingsData.win_key().supported());
    ui->groupBox_OtherSettings->setVisible(false);

    // Update UI from current data
    if(m_otherSettingsData.touch_pad().supported())
    {
        ui->checkBox_DisableTouchpad->setChecked(m_otherSettingsData.touch_pad().current());
        ui->checkBox_DisableTouchpad->blockSignals(false);
        ui->groupBox_OtherSettings->setVisible(true);
    }

    if(m_otherSettingsData.win_key().supported())
    {
        ui->checkBox_DisableWinKey->setChecked(m_otherSettingsData.win_key().current());
        ui->checkBox_DisableWinKey->blockSignals(false);
        ui->groupBox_OtherSettings->setVisible(true);
    }
}

void OtherControl::applySettings()
{
    LOG_T("OtherControl: Applying settings");

    // Create message with new settings
    legion::messages::OtherSettings newSettings;


    if(!ui->checkBox_DisableTouchpad->isHidden())
    {
        newSettings.mutable_touch_pad()->set_current(ui->checkBox_DisableTouchpad->isChecked());
    }

    if(!ui->checkBox_DisableWinKey->isHidden())
    {
        newSettings.mutable_win_key()->set_current(ui->checkBox_DisableWinKey->isChecked());
    }

    // Send to daemon
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderOther::dataType, newSettings);

    // Refresh data from daemon
    m_otherSettingsData = m_dataProvider->getDataMessage<legion::messages::OtherSettings>(
        LenovoLegionDaemon::SysFsDataProviderOther::dataType);

    // Re-render UI
    renderData();
}

void OtherControl::on_checkBox_DisableTouchpad_stateChanged(int)
{
    applySettings();
}

void OtherControl::on_checkBox_DisableWinKey_stateChanged(int)
{
    applySettings();
}

void OtherControl::renderGpuSwitchData()
{
    // Block signals to prevent triggering applyGpuSwitchSettings during refresh
    ui->radioButton_HybridOn->blockSignals(true);
    ui->radioButton_HybridAuto->blockSignals(true);
    ui->radioButton_HybridIGPUOnly->blockSignals(true);
    ui->radioButton_HybridOff->blockSignals(true);

    ui->groupBox_GPUControl->setVisible(m_gpuSwitchData.supported());

    if(m_gpuSwitchData.supported()) {
        // Update UI from current data
        if(m_gpuSwitchData.has_current()) {
            switch(m_gpuSwitchData.current()) {
            case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON:
                ui->radioButton_HybridOn->setChecked(true);
                break;
            case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO:
                ui->radioButton_HybridAuto->setChecked(true);
                break;
            case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY:
                ui->radioButton_HybridIGPUOnly->setChecked(true);
                break;
            case legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF:
                ui->radioButton_HybridOff->setChecked(true);
                break;
            default:
                break;
            }
        }

        // Unblock signals
        ui->radioButton_HybridOn->blockSignals(false);
        ui->radioButton_HybridAuto->blockSignals(false);
        ui->radioButton_HybridIGPUOnly->blockSignals(false);
        ui->radioButton_HybridOff->blockSignals(false);
    }
}

void OtherControl::renderCPUPstate()
{
    bool isVisible = false;
    bool refreshCPUControl = false;

    ui->checkBox_CPUPStatehwpDynamicBoost->blockSignals(true);
    ui->checkBox_CPUPStatedisableTurbo->blockSignals(true);
    ui->spinBox_CPUPState_MINLimit->blockSignals(true);
    ui->spinBox_CPUPState_MAXLimit->blockSignals(true);
    ui->comboBox_CPUPstate_status->blockSignals(true);
    ui->pushButton_CPUPState_Apply->blockSignals(true);
    ui->pushButton_CPUPState_Default->blockSignals(true);
    ui->pushButton_CPUPState_Cancel->blockSignals(true);
    ui->groupBox_CPUPState->blockSignals(true);




    ui->checkBox_CPUPStatehwpDynamicBoost->setVisible(false);
    ui->checkBox_CPUPStatedisableTurbo->setVisible(false);
    ui->comboBox_CPUPstate_status->setVisible(false);
    ui->spinBox_CPUPState_MINLimit->setVisible(false);
    ui->spinBox_CPUPState_MAXLimit->setVisible(false);
    ui->pushButton_CPUPState_Apply->setVisible(false);
    ui->pushButton_CPUPState_Default->setVisible(false);
    ui->pushButton_CPUPState_Cancel->setVisible(false);
    ui->groupBox_CPUPState->setVisible(false);

    if(m_cpuPStateData.has_hwp_dynamic_boost()) {
        ui->checkBox_CPUPStatehwpDynamicBoost->setVisible(true);
        ui->checkBox_CPUPStatehwpDynamicBoost->setChecked(m_cpuPStateData.hwp_dynamic_boost());
        ui->checkBox_CPUPStatehwpDynamicBoost->blockSignals(false);

        ui->groupBox_CPUPState->setVisible(true);
        isVisible = true;
    }

    if(m_cpuPStateData.has_no_turbo()) {
        ui->checkBox_CPUPStatedisableTurbo->setVisible(true);
        ui->checkBox_CPUPStatedisableTurbo->setChecked(m_cpuPStateData.no_turbo());
        ui->checkBox_CPUPStatedisableTurbo->blockSignals(false);

        ui->groupBox_CPUPState->setVisible(true);
        isVisible = true;
    }

    if(m_cpuPStateData.has_min_perf_pct()) {
        ui->spinBox_CPUPState_MINLimit->setVisible(true);
        ui->spinBox_CPUPState_MINLimit->setValue(m_cpuPStateData.min_perf_pct());
        ui->spinBox_CPUPState_MINLimit->blockSignals(false);

        ui->groupBox_CPUPState->setVisible(true);
        isVisible = true;
    }

    if(m_cpuPStateData.has_max_perf_pct()) {
        ui->spinBox_CPUPState_MAXLimit->setVisible(true);
        ui->spinBox_CPUPState_MAXLimit->setValue(m_cpuPStateData.max_perf_pct());
        ui->spinBox_CPUPState_MAXLimit->blockSignals(false);

        ui->groupBox_CPUPState->setVisible(true);
        isVisible = true;
    }

    if(m_cpuPStateData.has_status()) {
        ui->comboBox_CPUPstate_status->setVisible(true);

        if(ui->comboBox_CPUPstate_status->count() == 0)
        {
            ui->comboBox_CPUPstate_status->addItems({"active","passive"});
        }
        else
        {
            if(ui->comboBox_CPUPstate_status->currentText() != QString::fromStdString(std::string(m_cpuPStateData.status())))
            {
                refreshCPUControl = true;
            }
        }

        ui->comboBox_CPUPstate_status->setCurrentText(QString::fromStdString(std::string(m_cpuPStateData.status())));
        ui->comboBox_CPUPstate_status->blockSignals(false);

        ui->groupBox_CPUPState->setVisible(true);
        isVisible = true;
    }

    if(isVisible)
    {
        ui->pushButton_CPUPState_Apply->setVisible(true);
        ui->pushButton_CPUPState_Default->setVisible(true);
        ui->pushButton_CPUPState_Cancel->setVisible(true);

        ui->pushButton_CPUPState_Apply->blockSignals(false);
        ui->pushButton_CPUPState_Default->blockSignals(false);
        ui->pushButton_CPUPState_Cancel->blockSignals(false);
        ui->groupBox_CPUPState->blockSignals(false);
    }

    if(refreshCPUControl)
    {
        emit widgetEvent(WidgetMessage(WidgetMessage::Widget::CPU_CONTROL, WidgetMessage::Message::CPU_CONTROL_CHANGED));
    }
}

void OtherControl::applyGpuSwitchSettings()
{
    LOG_T("OtherControl: Applying GPU switch settings");

    // Create message with new settings
    legion::messages::GpuSwitchValue newSettings;

    if(ui->radioButton_HybridOn->isChecked()) {
        newSettings.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON);
    } else if(ui->radioButton_HybridAuto->isChecked()) {
        newSettings.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO);
    } else if(ui->radioButton_HybridIGPUOnly->isChecked()) {
        newSettings.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY);
    } else if(ui->radioButton_HybridOff->isChecked()) {
        newSettings.set_current(legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF);
    }

    // Send to daemon
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderOtherGpuSwitch::dataType, newSettings);


    // Check if restart is needed
    if(isRestartNeededForGpuSwitch(newSettings))
    {

        QMessageBox restartMsgBox(this);
        QPixmap dialogIcon(":/images/icons/dialog.png");
        restartMsgBox.setIconPixmap(dialogIcon.scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        restartMsgBox.setWindowTitle("Restart Required");
        restartMsgBox.setText("Changing the GPU mode requires a system restart to take effect. Please restart your computer to apply the new GPU settings.");
        restartMsgBox.setStandardButtons(QMessageBox::Ok);
        restartMsgBox.exec();
    }

    // Refresh data from daemon
    m_gpuSwitchData = m_dataProvider->getDataMessage<legion::messages::GpuSwitchValue>(
        LenovoLegionDaemon::SysFsDataProviderOtherGpuSwitch::dataType);


    // Re-render UI
    renderGpuSwitchData();
}

void OtherControl::applyCPUPStateSettings()
{
    bool refreshCPUControl = false;

    LOG_T("OtherControl: Applying CPU P-State settings");

    // Create message with new settings
    legion::messages::CPUPState newSettings;

    if(ui->checkBox_CPUPStatehwpDynamicBoost->isChecked() != m_cpuPStateData.hwp_dynamic_boost())
    {
        newSettings.set_hwp_dynamic_boost(ui->checkBox_CPUPStatehwpDynamicBoost->isChecked());
    }

    if(ui->checkBox_CPUPStatedisableTurbo->isChecked() != m_cpuPStateData.no_turbo())
    {
        newSettings.set_no_turbo(ui->checkBox_CPUPStatedisableTurbo->isChecked());
    }

    if(static_cast<uint32_t>(ui->spinBox_CPUPState_MINLimit->value()) != m_cpuPStateData.min_perf_pct())
    {
        newSettings.set_min_perf_pct(static_cast<uint32_t>(ui->spinBox_CPUPState_MINLimit->value()));
    }

    if(static_cast<uint32_t>(ui->spinBox_CPUPState_MAXLimit->value()) != m_cpuPStateData.max_perf_pct())
    {
        newSettings.set_max_perf_pct(static_cast<uint32_t>(ui->spinBox_CPUPState_MAXLimit->value()));
    }

    if(ui->comboBox_CPUPstate_status->currentText().toStdString() != m_cpuPStateData.status())
    {
        newSettings.set_status(ui->comboBox_CPUPstate_status->currentText().toStdString());
        refreshCPUControl = true;
    }

    if(newSettings.has_hwp_dynamic_boost()  ||
       newSettings.has_no_turbo()           ||
       newSettings.has_min_perf_pct()       ||
       newSettings.has_max_perf_pct()       ||
       newSettings.has_status())
    {
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUPState::dataType, newSettings);

        // Refresh data from daemon
        m_cpuPStateData = m_dataProvider->getDataMessage<legion::messages::CPUPState>(
            LenovoLegionDaemon::SysFsDataProviderCPUPState::dataType);

        // Re-render UI
        renderCPUPstate();

        LOG_T("OtherControl: CPU P-State settings applied");
    }

    if(refreshCPUControl)
    {
        emit widgetEvent(WidgetMessage(WidgetMessage::Widget::CPU_CONTROL, WidgetMessage::Message::CPU_CONTROL_CHANGED));
    }
}

void OtherControl::markCPUPStateSettingsChanged()
{
    if(ui->checkBox_CPUPStatedisableTurbo->isChecked() != m_cpuPStateData.no_turbo())
    {
        ui->checkBox_CPUPStatedisableTurbo->setStyleSheet(QString("QCheckBox {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->checkBox_CPUPStatedisableTurbo->setStyleSheet(QString("QCheckBox { }"));
    }

    if(ui->checkBox_CPUPStatehwpDynamicBoost->isChecked() != m_cpuPStateData.hwp_dynamic_boost())
    {
        ui->checkBox_CPUPStatehwpDynamicBoost->setStyleSheet(QString("QCheckBox {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->checkBox_CPUPStatehwpDynamicBoost->setStyleSheet(QString("QCheckBox { }"));
    }

    if(static_cast<uint32_t>(ui->spinBox_CPUPState_MINLimit->value()) != m_cpuPStateData.min_perf_pct())
    {
        ui->label_CPUPState_MINLimit->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_CPUPState_MINLimit->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->spinBox_CPUPState_MAXLimit->value()) != m_cpuPStateData.max_perf_pct())
    {
        ui->label_CPUPState_MAXLimit->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_CPUPState_MAXLimit->setStyleSheet(QString("QLabel { }"));
    }
}

bool OtherControl::isRestartNeededForGpuSwitch(const legion::messages::GpuSwitchValue& newSettings)
{
    if(m_gpuSwitchData.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON     ||
       m_gpuSwitchData.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO ||
       m_gpuSwitchData.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY)
    {
        return newSettings.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF;
    }

    if(m_gpuSwitchData.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_OFF)
    {
        return newSettings.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ON     ||
               newSettings.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONAUTO ||
               newSettings.current() == legion::messages::GpuSwitchValue_HybridModeState_HYBRID_MODE_ONIGPUONLY;
    };

    return false;
}

void OtherControl::on_radioButton_HybridOn_clicked()
{
    applyGpuSwitchSettings();
}

void OtherControl::on_radioButton_HybridAuto_clicked()
{
    applyGpuSwitchSettings();
}

void OtherControl::on_radioButton_HybridIGPUOnly_clicked()
{
    applyGpuSwitchSettings();
}

void OtherControl::on_radioButton_HybridOff_clicked()
{
    applyGpuSwitchSettings();
}

void OtherControl::readData()
{
    /*
     * Read data
     */
    m_otherSettingsData = m_dataProvider->getDataMessage<legion::messages::OtherSettings>(
        LenovoLegionDaemon::SysFsDataProviderOther::dataType);
    m_gpuSwitchData = m_dataProvider->getDataMessage<legion::messages::GpuSwitchValue>(
        LenovoLegionDaemon::SysFsDataProviderOtherGpuSwitch::dataType);
    m_cpuPStateData = m_dataProvider->getDataMessage<legion::messages::CPUPState>(
        LenovoLegionDaemon::SysFsDataProviderCPUPState::dataType);

    /*
     * Check data availability
     */
    if( !m_otherSettingsData.has_touch_pad()        &&
        !m_otherSettingsData.has_win_key()          &&
        !m_gpuSwitchData.has_current()              &&
        !m_cpuPStateData.has_hwp_dynamic_boost()    &&
        !m_cpuPStateData.has_min_perf_pct()         &&
        !m_cpuPStateData.has_max_perf_pct()         &&
        !m_cpuPStateData.has_no_turbo()             &&
        !m_cpuPStateData.has_status()
        )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "Other Settings data not available");
    }
}

void OtherControl::on_spinBox_CPUPState_MINLimit_valueChanged(int arg1)
{
    if(arg1 < 14)
    {
        ui->spinBox_CPUPState_MINLimit->setValue(14);
        return;
    }

    ui->spinBox_CPUPState_MAXLimit->setValue(std::max(arg1, ui->spinBox_CPUPState_MAXLimit->value()));
    markCPUPStateSettingsChanged();
}

void OtherControl::on_spinBox_CPUPState_MAXLimit_valueChanged(int arg1)
{
    if(arg1 < 14)
    {
        ui->spinBox_CPUPState_MAXLimit->setValue(14);
        return;
    }

    ui->spinBox_CPUPState_MINLimit->setValue(std::min(arg1, ui->spinBox_CPUPState_MINLimit->value()));
    markCPUPStateSettingsChanged();
}

void OtherControl::on_pushButton_CPUPState_Default_clicked()
{
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUPState::dataType, CPU_PSTATE_PRESET.value("Default"));

    // Refresh data from daemon
    m_cpuPStateData = m_dataProvider->getDataMessage<legion::messages::CPUPState>(
        LenovoLegionDaemon::SysFsDataProviderCPUPState::dataType);

    renderCPUPstate();

    markCPUPStateSettingsChanged();
}

void OtherControl::on_pushButton_CPUPState_Cancel_clicked()
{
    renderCPUPstate();
    markCPUPStateSettingsChanged();
}

void OtherControl::on_pushButton_CPUPState_Apply_clicked()
{
    applyCPUPStateSettings();
    markCPUPStateSettingsChanged();
}

void OtherControl::on_comboBox_CPUPstate_status_currentTextChanged(const QString &)
{
    applyCPUPStateSettings();
    markCPUPStateSettingsChanged();
}

void OtherControl::on_checkBox_CPUPStatedisableTurbo_stateChanged(int)
{
    markCPUPStateSettingsChanged();
}

void OtherControl::on_checkBox_CPUPStatehwpDynamicBoost_stateChanged(int)
{
    markCPUPStateSettingsChanged();
}

}
