// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "WidgetMessage.h"
#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-PrepareBuild/CpuPower.pb.h"
#include "../LenovoLegion-PrepareBuild/GPUPower.pb.h"
#include "../LenovoLegion-PrepareBuild/PowerProfile.pb.h"

#include <QWidget>
#include <QString>
#include <QMap>

namespace Ui {
class PowerControl;
}

namespace LenovoLegionGui {

class DataProvider;

class PowerControl : public QWidget
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(PowerControl)

    enum ERROR_CODES : int {
        DATA_NOT_READY = -1
    };

public:

    static const QString NAME;

public:

    explicit PowerControl(DataProvider *dataProvider , QWidget *parent = nullptr);

    ~PowerControl();

    void refresh();

signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private slots:

    /*
     * CPU
     */
    void on_comboBox_CPUBoostPowerControl_currentIndexChanged(int index);
    void on_comboBox_PL1TauPowerControl_currentIndexChanged(int index);
    void on_horizontalSlider_PowerTargetInACPowerControl_valueChanged(int value);
    void on_horizontalSlider_LTPowerLimitPowerControl_valueChanged(int value);
    void on_horizontalSlider_STPowerLimitPowerControl_valueChanged(int value);
    void on_horizontalSlider_CrossLPowerLimitPowerControl_valueChanged(int value);
    void on_horizontalSlider_CPUTempLimitPowerControl_valueChanged(int value);
    void on_pushButton_CPUPowerControlApply_clicked();
    void on_pushButton_CPUPwoerControlCancel_clicked();



    /*
     * GPU
     */
    void on_comboBox_GPUTargetPowerPowerControl_currentIndexChanged(int index);
    void on_comboBox_GPUBoostClockPowerControl_currentIndexChanged(int index);
    void on_horizontalSlider_GPUTempLimitPowerControl_valueChanged(int value);
    void on_pushButton_GPUPowerControlApply_clicked();
    void on_pushButton_GPUPowerControlCancel_clicked();


    void on_comboBox_PresetConfiguration_currentTextChanged(const QString &arg1);

private:

    void renderData();

    void renderCpuControlData();
    void renderGpuControlData();

    void markChanges();
    void markChangesCpuControlData();
    void markChangesGpuControlData();


private:

    Ui::PowerControl         *ui;
    DataProvider             *m_dataProvider;

    legion::messages::CPUPower             m_cpuControlData;
    legion::messages::GPUPower             m_gpuControlData;
    legion::messages::PowerProfile         m_powerProfileData;
};

}
