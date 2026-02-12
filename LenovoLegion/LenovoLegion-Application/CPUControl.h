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


#include "../LenovoLegion-PrepareBuild/CPUOptions.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUTopology.pb.h"

#include <QWidget>

namespace Ui {
class CPUControl;
}

namespace LenovoLegionGui {

class DataProvider;
class CPUControl : public QWidget
{
    Q_OBJECT

public:
    DEFINE_EXCEPTION(CPUControl)

    enum ERROR_CODES : int {
        DATA_NOT_READY = -1
    };

private:

    static const QMap<QString,legion::messages::CPUOptions::CPUX> CPU_CONTROL_PRESETS;

public:

    static const QString NAME;


    static const legion::messages::CPUSMT   SMT_ON_DATA;
    static const legion::messages::CPUSMT   SMT_OFF_DATA;

    static const QString APPLY_TO_ALL;
    static const QString APPLY_TO_ALL_ENABLED;
    static const QString APPLY_TO_ALL_EFFICIENT;
    static const QString APPLY_TO_ALL_PERFORMANCE;
    static const QString APPLY_TO_ALL_DISABLED;


    static constexpr std::string_view POWER_SAVE    = "POWER_SAVE";
    static constexpr std::string_view PERFORMANCE   = "PERFORMANCE";
    static constexpr std::string_view ONDEMAND      = "ONDEMAND";
    static constexpr std::string_view OFF           = "OFF";


public:
    explicit CPUControl(DataProvider *dataProvider,QWidget *parent = nullptr);
    ~CPUControl();

    void refresh();


    static legion::messages::CPUOptions::CPUX getCpuControlPreset(const QString &presetName,const legion::messages::CPUOptions::CPUX &dataInfo);


signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private slots:
    void on_pushButton_CPUControlEnableAll_clicked();

    void on_pushButton_CPUControlDisableAll_clicked();

    void on_comboBox_CPUGovernor_currentTextChanged(const QString &arg1);

    void on_pushButton_CPUControlCancel_clicked();

    void on_pushButton_CPUControlApply_clicked();

    void on_checkBox_DisableSMP_checkStateChanged(const Qt::CheckState &arg1);

    void on_comboBoxApplyTo_currentTextChanged(const QString &arg1);

private:

    void forAllCpuPerformanceCores(const std::function<bool(const int index)> &func);
    void forAllCpuEfficientCores(const std::function<bool(const int index)> &func);
    void forAllCpuDisabledCores(const std::function<bool(const int index)> &func);

    void renderData();

private:
    Ui::CPUControl *ui;

    DataProvider*  m_dataProvider;

    legion::messages::CPUOptions            m_cpuInfoData;
    legion::messages::CPUTopology           m_cpuTopology;
    legion::messages::CPUSMT                m_cpuSMTControlData;

};

}
