// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "PowerProfileControl.h"
#include "ui_PowerProfileControl.h"
#include "DataProvider.h"

#include "../LenovoLegion-PrepareBuild/MessageRegistry.pb.h"


namespace LenovoLegionGui {

PowerProfileControl::PowerProfileControl(DataProvider *dataProvider, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PowerProfileControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    /* Read Data */
    refresh();
}

PowerProfileControl::~PowerProfileControl()
{
    delete ui;
}

void PowerProfileControl::refresh()
{
    m_supportedProfiles.clear();

    m_powerProfileControlData = m_dataProvider->getDataMessage<legion::messages::PowerProfile > (legion::messages::DataType::POWER_PROFILE);
    m_batteryControlData      = m_dataProvider->getDataMessage<legion::messages::Battery> (legion::messages::DataType::BATTERY);

    for(int i = 0; i < m_powerProfileControlData.supported_profiles_size(); i++)
    {
        m_supportedProfiles.insert(m_powerProfileControlData.supported_profiles(i));
    }

    renderData();
}

void PowerProfileControl::setPowerProfile(legion::messages::PowerProfile::Profiles profile)
{
    legion::messages::PowerProfile data;

    data.set_current_value(profile);

    m_dataProvider->setDataMessage(legion::messages::DataType::POWER_PROFILE,data);

    /*
     * Re-read the real state, if the write fails no thermal mode event comes
     * and the selected button would not match the hardware
     */
    refresh();
}

void PowerProfileControl::on_radioButton_PPPerformance_clicked()
{
    setPowerProfile(legion::messages::PowerProfile::POWER_PROFILE_PERFORMANCE);
}

void PowerProfileControl::on_radioButton_PPCustom_clicked()
{
    setPowerProfile(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM);
}

void PowerProfileControl::on_radioButton_PPBalanced_clicked()
{
    setPowerProfile(legion::messages::PowerProfile::POWER_PROFILE_BALANCED);
}

void PowerProfileControl::on_radioButton_PPQuiet_clicked()
{
    setPowerProfile(legion::messages::PowerProfile::POWER_PROFILE_QUIET);
}

void PowerProfileControl::on_radioButton_PPExtreme_clicked()
{
    setPowerProfile(legion::messages::PowerProfile::POWER_PROFILE_EXTREME);
}

void PowerProfileControl::renderData()
{
    ui->groupBox->blockSignals(true);
    ui->checkBox_customFnQ->blockSignals(true);

    ui->groupBox->setVisible(false);
    ui->checkBox_customFnQ->setVisible(false);

    ui->radioButton_PPQuiet->setVisible(false);
    ui->radioButton_PPBalanced->setVisible(false);
    ui->radioButton_PPPerformance->setVisible(false);
    ui->radioButton_PPCustom->setVisible(false);
    ui->radioButton_PPExtreme->setVisible(false);

    if(!m_powerProfileControlData.has_current_value() || !m_powerProfileControlData.has_thermal_mode() || !m_powerProfileControlData.has_custom_fnq_enabled())
    {
        emit widgetEvent( LenovoLegionGui::WidgetMessage {
            .m_widget       = LenovoLegionGui::WidgetMessage::Widget::POWER_PROFILE_CONTROL,
            .m_message = LenovoLegionGui::WidgetMessage::Message::POWER_PROFILE_NOT_AVAILABLE
        });

        return;
    }

    ui->checkBox_customFnQ->setCheckState(Qt::Unchecked);

    switch (m_powerProfileControlData.thermal_mode()) {
    case legion::messages::PowerProfile::POWER_PROFILE_QUIET:
        ui->radioButton_PPQuiet->setChecked(true);
        break;
    case legion::messages::PowerProfile::POWER_PROFILE_BALANCED:
        ui->radioButton_PPBalanced->setChecked(true);
        break;
    case legion::messages::PowerProfile::POWER_PROFILE_PERFORMANCE:
        ui->radioButton_PPPerformance->setChecked(true);
        break;
    case legion::messages::PowerProfile::POWER_PROFILE_EXTREME:
        ui->radioButton_PPExtreme->setChecked(true);
        break;
    case legion::messages::PowerProfile::POWER_PROFILE_CUSTOM:
        ui->radioButton_PPCustom->setChecked(true);
        break;
    default:
        break;
    }

    if(m_powerProfileControlData.mutable_custom_fnq_enabled()->supported())
    {
        ui->checkBox_customFnQ->setChecked(m_powerProfileControlData.mutable_custom_fnq_enabled()->current_value());
        ui->checkBox_customFnQ->setVisible(true);
        ui->checkBox_customFnQ->blockSignals(false);
    }

    if(m_supportedProfiles.contains(legion::messages::PowerProfile::POWER_PROFILE_QUIET))
    {
        ui->radioButton_PPQuiet->setVisible(true);
    }
    if(m_supportedProfiles.contains(legion::messages::PowerProfile::POWER_PROFILE_BALANCED))
    {
        ui->radioButton_PPBalanced->setVisible(true);
    }

    if(m_batteryControlData.current_charge_mode_value() == legion::messages::Battery::POWER_CHARGE_MODE_AC)
    {
        if(m_supportedProfiles.contains(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM))
        {
            ui->radioButton_PPCustom->setVisible(true);
        }
        if(m_supportedProfiles.contains(legion::messages::PowerProfile::POWER_PROFILE_PERFORMANCE))
        {
            ui->radioButton_PPPerformance->setVisible(true);
        }
        if(m_supportedProfiles.contains(legion::messages::PowerProfile::POWER_PROFILE_EXTREME))
        {
            ui->radioButton_PPExtreme->setVisible(true);
        }
    }

    ui->groupBox->setVisible(true);
    ui->groupBox->blockSignals(false);

    emit widgetEvent( LenovoLegionGui::WidgetMessage {
        .m_widget       = LenovoLegionGui::WidgetMessage::Widget::POWER_PROFILE_CONTROL,
        .m_message = m_powerProfileControlData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM  ? LenovoLegionGui::WidgetMessage::Message::POWER_PROFILE_CHANGED_CUSTOM :LenovoLegionGui::WidgetMessage::Message::POWER_PROFILE_CHANGED
    });
}

void PowerProfileControl::on_checkBox_customFnQ_stateChanged(int arg1)
{
    legion::messages::PowerProfile data;

    data.mutable_custom_fnq_enabled()->set_current_value(arg1 != 0);

    m_dataProvider->setDataMessage(legion::messages::DataType::POWER_PROFILE,data);

    refresh();
}




}
