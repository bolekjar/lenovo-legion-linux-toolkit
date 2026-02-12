// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "PowerControl.h"
#include "ui_PowerControl.h"
#include "DataProvider.h"

#include <MainWindow.h>

#include "../LenovoLegion-Daemon/SysFsDataProviderCPUPower.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderGPUPower.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderPowerProfile.h"



#include <QStringList>
#include <QList>
#include <QAbstractItemView>

namespace LenovoLegionGui {


struct __attribute__ ((__packed__)) Data {
    quint8 m_shortTermPowerLimit;
    quint8 m_longTermPowerLimit;
    quint8 m_crossLoadingPowerLimit;
    quint8 m_tempLimit;
    quint8 m_l1Tau;
    quint8 m_totalPowerTargetInAC;
    quint8 m_powerBoost;
};

const QString PowerControl::NAME = "Power Control";



PowerControl::PowerControl(DataProvider *dataProvider,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PowerControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);


    ui->comboBox_CPUBoostPowerControl->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_CPUBoostPowerControl->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBox_GPUBoostClockPowerControl->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_GPUBoostClockPowerControl->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBox_GPUTargetPowerPowerControl->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_GPUTargetPowerPowerControl->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBox_PL1TauPowerControl->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_PL1TauPowerControl->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBox_PresetConfiguration->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_PresetConfiguration->view()->window()->setAttribute(Qt::WA_TranslucentBackground);


    /*
     * Read Data
     */
    m_cpuControlData    = m_dataProvider->getDataMessage<legion::messages::CPUPower> (LenovoLegionDaemon::SysFsDataProviderCPUPower::dataType);
    m_gpuControlData    = m_dataProvider->getDataMessage<legion::messages::GPUPower> (LenovoLegionDaemon::SysFsDataProviderGPUPower::dataType);
    m_powerProfileData  = m_dataProvider->getDataMessage<legion::messages::PowerProfile> (LenovoLegionDaemon::SysFsDataProviderPowerProfile::dataType);


    if(!m_cpuControlData.has_cpu_clp_limit()            ||
       !m_cpuControlData.has_cpu_ltp_limit()            ||
       !m_cpuControlData.has_cpu_stp_limit()            ||
       !m_cpuControlData.has_cpu_tmp_limit()            ||
       !m_cpuControlData.has_gpu_to_cpu_dynamic_boost() ||
       !m_cpuControlData.has_gpu_total_onac()           ||
       !m_gpuControlData.has_gpu_configurable_tgp()     ||
       !m_gpuControlData.has_gpu_power_boost()          ||
       !m_gpuControlData.has_gpu_temperature_limit()    ||
       !m_powerProfileData.has_thermal_mode()
       )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "Power Control data not available");
    }

    auto qlistToQStringList = [] (const QList<int>& list) {
        QStringList stringList;
        for (const auto& item : list) {
            stringList.append(QString::number(item).trimmed());
        }
        return stringList;
    };



    /*
     * Setup GUI elements
     */
    ui->comboBox_CPUBoostPowerControl->blockSignals(true);
    ui->comboBox_PL1TauPowerControl->blockSignals(true);
    ui->comboBox_GPUTargetPowerPowerControl->blockSignals(true);
    ui->comboBox_GPUBoostClockPowerControl->blockSignals(true);



    ui->horizontalSlider_PowerTargetInACPowerControl->blockSignals(true);
    ui->horizontalSlider_LTPowerLimitPowerControl->blockSignals(true);
    ui->horizontalSlider_STPowerLimitPowerControl->blockSignals(true);
    ui->horizontalSlider_CrossLPowerLimitPowerControl->blockSignals(true);
    ui->horizontalSlider_CPUTempLimitPowerControl->blockSignals(true);
    ui->horizontalSlider_GPUTempLimitPowerControl->blockSignals(true);
    ui->comboBox_PresetConfiguration->blockSignals(true);


    ui->comboBox_CPUBoostPowerControl->setVisible(false);
    ui->comboBox_PL1TauPowerControl->setVisible(false);
    ui->comboBox_GPUTargetPowerPowerControl->setVisible(false);
    ui->comboBox_GPUBoostClockPowerControl->setVisible(false);



    ui->horizontalSlider_PowerTargetInACPowerControl->setVisible(false);
    ui->horizontalSlider_LTPowerLimitPowerControl->setVisible(false);
    ui->horizontalSlider_STPowerLimitPowerControl->setVisible(false);
    ui->horizontalSlider_CrossLPowerLimitPowerControl->setVisible(false);
    ui->horizontalSlider_CPUTempLimitPowerControl->setVisible(false);
    ui->horizontalSlider_GPUTempLimitPowerControl->setVisible(false);
    ui->comboBox_PresetConfiguration->setVisible(false);


    ui->lcdNumber_CPUTempLimitPowerControl->setVisible(false);
    ui->lcdNumber_GPUTempLimitPowerControl->setVisible(false);
    ui->lcdNumber_LTPowerLimitPowerControl->setVisible(false);
    ui->lcdNumber_STPowerLimitPowerControl->setVisible(false);
    ui->lcdNumber_CrossLPowerLimitPowerControl->setVisible(false);
    ui->lcdNumber_PowerTargetInACPowerControl->setVisible(false);

    ui->label_GPUTargetPowerPowerControl->setVisible(false);
    ui->label_GPUBoostClockPowerControl->setVisible(false);
    ui->label_PresetConfiguration->setVisible(false);
    ui->label_PL1TauPowerControl->setVisible(false);
    ui->label_CPUBoostPowerControl->setVisible(false);
    ui->label_CPUTempLimitPowerControl->setVisible(false);
    ui->label_GPUTempLimitPowerControl->setVisible(false);
    ui->label_LTPowerLimitPowerControl->setVisible(false);
    ui->label_STPowerLimitPowerControl->setVisible(false);
    ui->label_CrossLPowerLimitPowerControl->setVisible(false);
    ui->label_PowerTargetInACPowerControl->setVisible(false);

    ui->pushButton_CPUPowerControlApply->setVisible(false);
    ui->pushButton_CPUPwoerControlCancel->setVisible(false);
    ui->pushButton_GPUPowerControlApply->setVisible(false);
    ui->pushButton_GPUPowerControlCancel->setVisible(false);
    ui->groupBox_CPUPowerControl->setVisible(false);
    ui->groupBox_GPUPowerControl->setVisible(false);



    if(m_cpuControlData.gpu_to_cpu_dynamic_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {

        ui->comboBox_CPUBoostPowerControl->clear();
        ui->comboBox_CPUBoostPowerControl->addItems(qlistToQStringList(QList<int>(m_cpuControlData.gpu_to_cpu_dynamic_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().begin(),
                                                                                  m_cpuControlData.gpu_to_cpu_dynamic_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().end()
                                                                                  )
                                                                       )
                                                    );

        ui->comboBox_CPUBoostPowerControl->setVisible(true);
        ui->label_CPUBoostPowerControl->setVisible(true);
        ui->comboBox_CPUBoostPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.cpu_pl1_tau().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {

        ui->comboBox_PL1TauPowerControl->clear();
        ui->comboBox_PL1TauPowerControl->addItems(qlistToQStringList(QList<int>(m_cpuControlData.cpu_pl1_tau().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().begin(),
                                                                                m_cpuControlData.cpu_pl1_tau().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().end()
                                                                                )
                                                                     )
                                                  );
        ui->comboBox_PL1TauPowerControl->setVisible(true);
        ui->label_PL1TauPowerControl->setVisible(true);
        ui->comboBox_PL1TauPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }



    if(m_gpuControlData.gpu_configurable_tgp().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->comboBox_GPUTargetPowerPowerControl->clear();
        ui->comboBox_GPUTargetPowerPowerControl->addItems(qlistToQStringList(QList<int>(m_gpuControlData.gpu_configurable_tgp().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().begin(),
                                                                                        m_gpuControlData.gpu_configurable_tgp().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().end()
                                                                                        )
                                                                             )
                                                          );

        ui->comboBox_GPUTargetPowerPowerControl->setVisible(true);
        ui->label_GPUTargetPowerPowerControl->setVisible(true);
        ui->comboBox_GPUTargetPowerPowerControl->blockSignals(false);

        ui->pushButton_GPUPowerControlApply->setVisible(true);
        ui->pushButton_GPUPowerControlCancel->setVisible(true);
        ui->groupBox_GPUPowerControl->setVisible(true);

    }


    if(m_gpuControlData.gpu_power_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->comboBox_GPUBoostClockPowerControl->clear();
        ui->comboBox_GPUBoostClockPowerControl->addItems(qlistToQStringList(QList<int>(m_gpuControlData.gpu_power_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().begin(),
                                                                                       m_gpuControlData.gpu_power_boost().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).steps().end()
                                                                                       )
                                                                            )
                                                         );

        ui->comboBox_GPUBoostClockPowerControl->setVisible(true);
        ui->label_GPUBoostClockPowerControl->setVisible(true);
        ui->comboBox_GPUBoostClockPowerControl->blockSignals(false);

        ui->pushButton_GPUPowerControlApply->setVisible(true);
        ui->pushButton_GPUPowerControlCancel->setVisible(true);
        ui->groupBox_GPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {

        ui->horizontalSlider_PowerTargetInACPowerControl->setMinimum(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_PowerTargetInACPowerControl->setMaximum(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_PowerTargetInACPowerControl->setVisible(true);
        ui->lcdNumber_PowerTargetInACPowerControl->setVisible(true);
        ui->label_PowerTargetInACPowerControl->setVisible(true);
        ui->horizontalSlider_PowerTargetInACPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->horizontalSlider_LTPowerLimitPowerControl->setMinimum(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_LTPowerLimitPowerControl->setMaximum(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_LTPowerLimitPowerControl->setVisible(true);
        ui->lcdNumber_LTPowerLimitPowerControl->setVisible(true);
        ui->label_LTPowerLimitPowerControl->setVisible(true);
        ui->horizontalSlider_LTPowerLimitPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->horizontalSlider_STPowerLimitPowerControl->setMinimum(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_STPowerLimitPowerControl->setMaximum(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_STPowerLimitPowerControl->setVisible(true);
        ui->lcdNumber_STPowerLimitPowerControl->setVisible(true);
        ui->label_STPowerLimitPowerControl->setVisible(true);
        ui->horizontalSlider_STPowerLimitPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->horizontalSlider_CrossLPowerLimitPowerControl->setMinimum(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_CrossLPowerLimitPowerControl->setMaximum(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_CrossLPowerLimitPowerControl->setVisible(true);
        ui->lcdNumber_CrossLPowerLimitPowerControl->setVisible(true);
        ui->label_CrossLPowerLimitPowerControl->setVisible(true);
        ui->horizontalSlider_CrossLPowerLimitPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->horizontalSlider_CPUTempLimitPowerControl->setMinimum(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_CPUTempLimitPowerControl->setMaximum(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_CPUTempLimitPowerControl->setVisible(true);
        ui->lcdNumber_CPUTempLimitPowerControl->setVisible(true);
        ui->label_CPUTempLimitPowerControl->setVisible(true);
        ui->horizontalSlider_CPUTempLimitPowerControl->blockSignals(false);
        ui->pushButton_CPUPowerControlApply->setVisible(true);
        ui->pushButton_CPUPwoerControlCancel->setVisible(true);
        ui->groupBox_CPUPowerControl->setVisible(true);
    }


    if(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).supported())
    {
        ui->horizontalSlider_GPUTempLimitPowerControl->setMinimum(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).min_value());
        ui->horizontalSlider_GPUTempLimitPowerControl->setMaximum(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(legion::messages::PowerProfile::POWER_PROFILE_CUSTOM).max_value());

        ui->horizontalSlider_GPUTempLimitPowerControl->setVisible(true);
        ui->lcdNumber_GPUTempLimitPowerControl->setVisible(true);
        ui->label_GPUTempLimitPowerControl->setVisible(true);
        ui->horizontalSlider_GPUTempLimitPowerControl->blockSignals(false);
        ui->pushButton_GPUPowerControlApply->setVisible(true);
        ui->pushButton_GPUPowerControlCancel->setVisible(true);
        ui->groupBox_GPUPowerControl->setVisible(true);
    }


    ui->comboBox_PresetConfiguration->addItem("NONE");
    for (auto clpI = m_cpuControlData.cpu_clp_limit().mode_descriptor_map().begin(); clpI != m_cpuControlData.cpu_clp_limit().mode_descriptor_map().end(); ++clpI)
    {
        ui->comboBox_PresetConfiguration->addItem(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByNumber(clpI->first)->name().data());
    }
    ui->comboBox_PresetConfiguration->setCurrentText("NONE");
    ui->comboBox_PresetConfiguration->setVisible(true);
    ui->label_PresetConfiguration->setVisible(true);
    ui->comboBox_PresetConfiguration->blockSignals(false);
    ui->groupBox_GPUPowerControl->setVisible(true);


    renderData();
    markChanges();
}

PowerControl::~PowerControl()
{
    delete ui;
}

void PowerControl::refresh()
{
    renderData();
}

void PowerControl::on_pushButton_CPUPowerControlApply_clicked()
{
    legion::messages::CPUPower cpuPower;

    if(!ui->horizontalSlider_CrossLPowerLimitPowerControl->isHidden())
    {
        cpuPower.mutable_cpu_clp_limit()->set_current_value((quint8)ui->horizontalSlider_CrossLPowerLimitPowerControl->value());
    }

    if(!ui->horizontalSlider_LTPowerLimitPowerControl->isHidden())
    {
        cpuPower.mutable_cpu_ltp_limit()->set_current_value((quint8)ui->horizontalSlider_LTPowerLimitPowerControl->value());
    }

    if(!ui->horizontalSlider_STPowerLimitPowerControl->isHidden())
    {
        cpuPower.mutable_cpu_stp_limit()->set_current_value((quint8)ui->horizontalSlider_STPowerLimitPowerControl->value());
    }


    if(!ui->horizontalSlider_CPUTempLimitPowerControl->isHidden())
    {
        cpuPower.mutable_cpu_tmp_limit()->set_current_value((quint8)ui->horizontalSlider_CPUTempLimitPowerControl->value());
    }


    if(!ui->horizontalSlider_PowerTargetInACPowerControl->isHidden())
    {
        cpuPower.mutable_gpu_total_onac()->set_current_value((quint8)ui->horizontalSlider_PowerTargetInACPowerControl->value());
    }


    if(!ui->comboBox_CPUBoostPowerControl->isHidden())
    {
        cpuPower.mutable_gpu_to_cpu_dynamic_boost()->set_current_value((quint8)ui->comboBox_CPUBoostPowerControl->currentText().toUShort());
    }

    if(!ui->comboBox_PL1TauPowerControl->isHidden())
    {
        cpuPower.mutable_cpu_pl1_tau()->set_current_value((quint8)ui->comboBox_PL1TauPowerControl->currentText().toUShort());
    }


    //CPU Power Control Apply
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUPower::dataType,cpuPower);

    m_cpuControlData = m_dataProvider->getDataMessage<legion::messages::CPUPower> (LenovoLegionDaemon::SysFsDataProviderCPUPower::dataType);


    renderCpuControlData();
    markChangesCpuControlData();
}

void PowerControl::on_pushButton_CPUPwoerControlCancel_clicked()
{
    renderCpuControlData();
}

void PowerControl::on_comboBox_CPUBoostPowerControl_currentIndexChanged(int)
{
    markChangesCpuControlData();
}

void PowerControl::on_comboBox_PL1TauPowerControl_currentIndexChanged(int)
{
    markChangesCpuControlData();
}

void PowerControl::on_horizontalSlider_PowerTargetInACPowerControl_valueChanged(int value)
{
    ui->lcdNumber_PowerTargetInACPowerControl->display(value);
    markChangesCpuControlData();
}

void PowerControl::on_horizontalSlider_LTPowerLimitPowerControl_valueChanged(int value)
{
    ui->lcdNumber_LTPowerLimitPowerControl->display(value);
    markChangesCpuControlData();
}

void PowerControl::on_horizontalSlider_STPowerLimitPowerControl_valueChanged(int value)
{
    ui->lcdNumber_STPowerLimitPowerControl->display(value);
    markChangesCpuControlData();
}

void PowerControl::on_horizontalSlider_CrossLPowerLimitPowerControl_valueChanged(int value)
{
    ui->lcdNumber_CrossLPowerLimitPowerControl->display(value);
    markChangesCpuControlData();
}

void PowerControl::on_horizontalSlider_CPUTempLimitPowerControl_valueChanged(int value)
{
    ui->lcdNumber_CPUTempLimitPowerControl->display(value);
    markChangesCpuControlData();
}

void PowerControl::on_comboBox_GPUTargetPowerPowerControl_currentIndexChanged(int)
{
    markChangesGpuControlData();
}

void PowerControl::on_comboBox_GPUBoostClockPowerControl_currentIndexChanged(int)
{
    markChangesGpuControlData();
}

void PowerControl::on_horizontalSlider_GPUTempLimitPowerControl_valueChanged(int value)
{
    ui->lcdNumber_GPUTempLimitPowerControl->display(value);
    markChangesGpuControlData();
}

void PowerControl::on_pushButton_GPUPowerControlApply_clicked()
{
    legion::messages::GPUPower gpuPower;

    if(!ui->comboBox_GPUBoostClockPowerControl->isHidden())
    {
        gpuPower.mutable_gpu_power_boost()->set_current_value((quint8)ui->comboBox_GPUBoostClockPowerControl->currentText().toUShort());
    }

    if(!ui->comboBox_GPUTargetPowerPowerControl->isHidden())
    {
        gpuPower.mutable_gpu_configurable_tgp()->set_current_value((quint8)ui->comboBox_GPUTargetPowerPowerControl->currentText().toUShort());
    }

    if(!ui->horizontalSlider_GPUTempLimitPowerControl->isHidden())
    {
        gpuPower.mutable_gpu_temperature_limit()->set_current_value((quint8)ui->horizontalSlider_GPUTempLimitPowerControl->value());
    }

    //GPU Power Control Apply
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderGPUPower::dataType,gpuPower);
    m_gpuControlData = m_dataProvider->getDataMessage<legion::messages::GPUPower> (LenovoLegionDaemon::SysFsDataProviderGPUPower::dataType);
    renderGpuControlData();
    markChangesGpuControlData();
}

void PowerControl::on_pushButton_GPUPowerControlCancel_clicked()
{
    renderGpuControlData();
}

void PowerControl::renderData()
{
    renderCpuControlData();
    renderGpuControlData();
}

void PowerControl::renderGpuControlData()
{
    ui->groupBox_GPUPowerControl->blockSignals(true);
    ui->groupBox_GPUPowerControl->setEnabled(false);
    ui->comboBox_PresetConfiguration->blockSignals(true);
    ui->comboBox_PresetConfiguration->setEnabled(false);
    ui->comboBox_PresetConfiguration->setCurrentText("NONE");

    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        if(!ui->comboBox_GPUTargetPowerPowerControl->isHidden())
        {
            ui->comboBox_GPUTargetPowerPowerControl->setCurrentIndex(ui->comboBox_GPUTargetPowerPowerControl->findText(QString::number(m_gpuControlData.gpu_configurable_tgp().current_value())));
        }

        if(!ui->comboBox_GPUBoostClockPowerControl->isHidden())
        {
            ui->comboBox_GPUBoostClockPowerControl->setCurrentIndex(ui->comboBox_GPUBoostClockPowerControl->findText(QString::number(m_gpuControlData.gpu_power_boost().current_value())));
        }


        if(!ui->horizontalSlider_GPUTempLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_GPUTempLimitPowerControl->setValue(m_gpuControlData.gpu_temperature_limit().current_value());
            ui->lcdNumber_GPUTempLimitPowerControl->display(static_cast<int>(m_gpuControlData.gpu_temperature_limit().current_value()));
        }

        ui->groupBox_GPUPowerControl->setEnabled(true);
        ui->groupBox_GPUPowerControl->blockSignals(false);
    }
    else
    {

        if(!ui->comboBox_GPUTargetPowerPowerControl->isHidden())
        {
            ui->comboBox_GPUTargetPowerPowerControl->setCurrentIndex(ui->comboBox_GPUTargetPowerPowerControl->findText(QString::number(m_gpuControlData.gpu_configurable_tgp().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value())));

        }


        if(!ui->comboBox_GPUBoostClockPowerControl->isHidden())
        {
            ui->comboBox_GPUBoostClockPowerControl->setCurrentIndex(ui->comboBox_GPUBoostClockPowerControl->findText(QString::number(m_gpuControlData.gpu_power_boost().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value())));

        }

        if(!ui->horizontalSlider_GPUTempLimitPowerControl->isHidden())
        {

            ui->horizontalSlider_GPUTempLimitPowerControl->setValue(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_GPUTempLimitPowerControl->display(static_cast<int>(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }
    }


    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        ui->comboBox_PresetConfiguration->setEnabled(true);
        ui->comboBox_PresetConfiguration->blockSignals(false);
    }
}

void PowerControl::markChanges()
{
    if(m_powerProfileData.thermal_mode() != legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        return;
    }

    markChangesCpuControlData();
    markChangesGpuControlData();
}

void PowerControl::markChangesCpuControlData()
{
    if(m_powerProfileData.thermal_mode() != legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        return;
    }


    if(static_cast<uint32_t>(ui->horizontalSlider_LTPowerLimitPowerControl->value()) != (m_cpuControlData.cpu_ltp_limit().current_value()))
    {
        ui->label_LTPowerLimitPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_LTPowerLimitPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->horizontalSlider_STPowerLimitPowerControl->value()) != (m_cpuControlData.cpu_stp_limit().current_value()))
    {
        ui->label_STPowerLimitPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_STPowerLimitPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->horizontalSlider_CrossLPowerLimitPowerControl->value()) != m_cpuControlData.cpu_clp_limit().current_value())
    {
        ui->label_CrossLPowerLimitPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_CrossLPowerLimitPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->horizontalSlider_CPUTempLimitPowerControl->value()) != m_cpuControlData.cpu_tmp_limit().current_value())
    {
        ui->label_CPUTempLimitPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_CPUTempLimitPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->comboBox_PL1TauPowerControl->currentText().toUShort()) != ((m_cpuControlData.cpu_pl1_tau().current_value())))
    {
        ui->label_PL1TauPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_PL1TauPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->horizontalSlider_PowerTargetInACPowerControl->value()) != m_cpuControlData.gpu_total_onac().current_value())
    {
        ui->label_PowerTargetInACPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_PowerTargetInACPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->comboBox_CPUBoostPowerControl->currentText().toUShort()) != m_cpuControlData.gpu_to_cpu_dynamic_boost().current_value())
    {
        ui->label_CPUBoostPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_CPUBoostPowerControl->setStyleSheet(QString("QLabel { }"));
    }
}

void PowerControl::markChangesGpuControlData()
{

    if(m_powerProfileData.thermal_mode() != legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        return;
    }


    if(static_cast<uint32_t>(ui->horizontalSlider_GPUTempLimitPowerControl->value()) != m_gpuControlData.gpu_temperature_limit().current_value())
    {
        ui->label_GPUTempLimitPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_GPUTempLimitPowerControl->setStyleSheet(QString("QLabel { }"));
    }


    if(static_cast<uint32_t>(ui->comboBox_GPUBoostClockPowerControl->currentText().toShort()) != m_gpuControlData.gpu_power_boost().current_value())
    {
        ui->label_GPUBoostClockPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_GPUBoostClockPowerControl->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<uint32_t>(ui->comboBox_GPUTargetPowerPowerControl->currentText().toUShort()) != m_gpuControlData.gpu_configurable_tgp().current_value())
    {
        ui->label_GPUTargetPowerPowerControl->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_GPUTargetPowerPowerControl->setStyleSheet(QString("QLabel { }"));
    }
}

void PowerControl::renderCpuControlData()
{
    ui->groupBox_CPUPowerControl->blockSignals(true);
    ui->groupBox_CPUPowerControl->setEnabled(false);
    ui->comboBox_PresetConfiguration->blockSignals(true);
    ui->comboBox_PresetConfiguration->setEnabled(false);
    ui->comboBox_PresetConfiguration->setCurrentText("NONE");

    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        if(!ui->horizontalSlider_LTPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_LTPowerLimitPowerControl->setValue(m_cpuControlData.cpu_ltp_limit().current_value());
            ui->lcdNumber_LTPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_ltp_limit().current_value()));
        }


        if(!ui->horizontalSlider_CrossLPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_CrossLPowerLimitPowerControl->setValue(m_cpuControlData.cpu_clp_limit().current_value());
            ui->lcdNumber_CrossLPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_clp_limit().current_value()));
        }


        if(!ui->horizontalSlider_STPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_STPowerLimitPowerControl->setValue(m_cpuControlData.cpu_stp_limit().current_value());
            ui->lcdNumber_STPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_stp_limit().current_value()));
        }


        if(!ui->horizontalSlider_CPUTempLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_CPUTempLimitPowerControl->setValue(m_cpuControlData.cpu_tmp_limit().current_value());
            ui->lcdNumber_CPUTempLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_tmp_limit().current_value()));
        }


        if(!ui->horizontalSlider_PowerTargetInACPowerControl->isHidden())
        {
            ui->horizontalSlider_PowerTargetInACPowerControl->setValue(m_cpuControlData.gpu_total_onac().current_value());
            ui->lcdNumber_PowerTargetInACPowerControl->display(static_cast<int>(m_cpuControlData.gpu_total_onac().current_value()));
        }


        if(!ui->comboBox_CPUBoostPowerControl->isHidden())
        {
            ui->comboBox_CPUBoostPowerControl->setCurrentIndex(ui->comboBox_CPUBoostPowerControl->findText(QString::number((m_cpuControlData.gpu_to_cpu_dynamic_boost().current_value()))));
        }

        if(!ui->comboBox_PL1TauPowerControl->isHidden())
        {
            ui->comboBox_PL1TauPowerControl->setCurrentIndex(ui->comboBox_PL1TauPowerControl->findText(QString::number((m_cpuControlData.cpu_pl1_tau().current_value()))));
        }

        ui->groupBox_CPUPowerControl->setEnabled(true);
        ui->groupBox_CPUPowerControl->blockSignals(false);
    }
    else
    {
        if(!ui->horizontalSlider_LTPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_LTPowerLimitPowerControl->setValue(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_LTPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }

        if(!ui->horizontalSlider_CrossLPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_CrossLPowerLimitPowerControl->setValue(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_CrossLPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }


        if(!ui->horizontalSlider_STPowerLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_STPowerLimitPowerControl->setValue(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_STPowerLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }


        if(!ui->horizontalSlider_CPUTempLimitPowerControl->isHidden())
        {
            ui->horizontalSlider_CPUTempLimitPowerControl->setValue(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_CPUTempLimitPowerControl->display(static_cast<int>(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }

        if(!ui->horizontalSlider_PowerTargetInACPowerControl->isHidden())
        {
            ui->horizontalSlider_PowerTargetInACPowerControl->setValue(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value());
            ui->lcdNumber_PowerTargetInACPowerControl->display(static_cast<int>(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value()));
        }


        if(!ui->comboBox_CPUBoostPowerControl->isHidden())
        {
            ui->comboBox_CPUBoostPowerControl->setCurrentIndex(ui->comboBox_CPUBoostPowerControl->findText(QString::number(m_cpuControlData.gpu_to_cpu_dynamic_boost().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value())));
        }

        if(!ui->comboBox_PL1TauPowerControl->isHidden())
        {
            ui->comboBox_PL1TauPowerControl->setCurrentIndex(ui->comboBox_PL1TauPowerControl->findText(QString::number(m_cpuControlData.cpu_pl1_tau().mode_descriptor_map().at(m_powerProfileData.current_value()).default_value())));

        }
    }

    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        ui->comboBox_PresetConfiguration->setEnabled(true);
        ui->comboBox_PresetConfiguration->blockSignals(false);
    }
}

void PowerControl::on_comboBox_PresetConfiguration_currentTextChanged(const QString &arg1)
{
    if(arg1 == "NONE")
        return;

    if(!ui->horizontalSlider_LTPowerLimitPowerControl->isHidden())
    {
        ui->horizontalSlider_LTPowerLimitPowerControl->setValue(m_cpuControlData.cpu_ltp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }

    if(!ui->horizontalSlider_STPowerLimitPowerControl->isHidden())
    {
        ui->horizontalSlider_STPowerLimitPowerControl->setValue(m_cpuControlData.cpu_stp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }

    if(!ui->horizontalSlider_CrossLPowerLimitPowerControl->isHidden())
    {
        ui->horizontalSlider_CrossLPowerLimitPowerControl->setValue(m_cpuControlData.cpu_clp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }

    if(!ui->horizontalSlider_CPUTempLimitPowerControl->isHidden())
    {
        ui->horizontalSlider_CPUTempLimitPowerControl->setValue(m_cpuControlData.cpu_tmp_limit().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }

    if(!ui->horizontalSlider_PowerTargetInACPowerControl->isHidden())
    {
       ui->horizontalSlider_PowerTargetInACPowerControl->setValue(m_cpuControlData.gpu_total_onac().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }

    if(!ui->comboBox_PL1TauPowerControl->isHidden())
    {
        ui->comboBox_PL1TauPowerControl->setCurrentIndex(ui->comboBox_PL1TauPowerControl->findText(QString::number(m_cpuControlData.cpu_pl1_tau().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value())));
    }


    if(!ui->comboBox_CPUBoostPowerControl->isHidden())
    {
       ui->comboBox_CPUBoostPowerControl->setCurrentIndex(ui->comboBox_CPUBoostPowerControl->findText(QString::number(m_cpuControlData.gpu_to_cpu_dynamic_boost().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value())));
    }

    if(!ui->comboBox_GPUTargetPowerPowerControl->isHidden())
    {
        ui->comboBox_GPUTargetPowerPowerControl->setCurrentIndex(ui->comboBox_GPUTargetPowerPowerControl->findText(QString::number(m_gpuControlData.gpu_configurable_tgp().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value())));
    }

    if(!ui->comboBox_GPUBoostClockPowerControl->isHidden())
    {
        ui->comboBox_GPUBoostClockPowerControl->setCurrentIndex(ui->comboBox_GPUBoostClockPowerControl->findText(QString::number(m_gpuControlData.gpu_power_boost().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value())));
    }

    if(!ui->horizontalSlider_GPUTempLimitPowerControl->isHidden())
    {
        ui->horizontalSlider_GPUTempLimitPowerControl->setValue(m_gpuControlData.gpu_temperature_limit().mode_descriptor_map().at(legion::messages::PowerProfile::Profiles_descriptor()->FindValueByName(arg1.toStdString().c_str())->number()).default_value());
    }
}


}
