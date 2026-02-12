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

#include "../LenovoLegion-Daemon/SysFsDataProviderOther.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderOtherGpuSwitch.h"

#include <Core/LoggerHolder.h>

#include <QMessageBox>

namespace LenovoLegionGui {

const QString OtherControl::NAME = "Other Settings";

OtherControl::OtherControl(DataProvider *dataProvider, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OtherControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    /*
     * Read data
     */
    m_otherSettingsData = m_dataProvider->getDataMessage<legion::messages::OtherSettings>(
        LenovoLegionDaemon::SysFsDataProviderOther::dataType);
    m_gpuSwitchData = m_dataProvider->getDataMessage<legion::messages::GpuSwitchValue>(
        LenovoLegionDaemon::SysFsDataProviderOtherGpuSwitch::dataType);

    /*
     * Check data availability
     */
    if(!m_otherSettingsData.has_touch_pad() ||
       !m_otherSettingsData.has_win_key()
        )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "Other Settings data not available");
    }

    /*
     * Render data
     */
    refresh();
}

OtherControl::~OtherControl()
{
    delete ui;
}

void OtherControl::refresh()
{
    renderData();
    renderGpuSwitchData();
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

}
