// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "CPUFrequencyControl.h"
#include "ui_CPUFrequencyControl.h"
#include "Utils.h"
#include "DataProvider.h"

#include "ThreadFrequencyControl.h"

#include "../LenovoLegion-Daemon/SysFsDataProviderCPUFrequency.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUTopology.h"


#include <QAbstractItemView>

namespace LenovoLegionGui {

QString const CPUFrequencyControl::NAME =  "CPU Frequency Control";


static legion::messages::CPUFrequency::CPUX getLegionCPUX(uint32_t scalingMin,uint32_t scalingMax){
    legion::messages::CPUFrequency::CPUX cpux;
    cpux.set_scaling_min_freq(scalingMin);
    cpux.set_scaling_max_freq(scalingMax);
    return cpux;
}


const QMap<QString,legion::messages::CPUFrequency::CPUX> CPUFrequencyControl::CPUFREQ_PRESETS ={
    {"MAX_MAX", getLegionCPUX(5600 * 1000,5600 * 1000)},
    {"MIN_MAX", getLegionCPUX(800  * 1000,5600 * 1000)},
    {"MIN_3000",getLegionCPUX(800  * 1000,3000 * 1000)},
    {"MIN_4000",getLegionCPUX(800  * 1000,4000 * 1000)},
    {"3000_3000",getLegionCPUX(3000 * 1000,3000 * 1000)},
    {"3500_3500",getLegionCPUX(3500 * 1000,3500 * 1000)},
    {"4000_4000",getLegionCPUX(4000 * 1000,4000 * 1000)},
    {"4500_4500",getLegionCPUX(4500 * 1000,500 * 1000)}
};

const QString CPUFrequencyControl::APPLY_TO_ALL = "All";
const QString CPUFrequencyControl::APPLY_TO_ALL_EFFICIENT = "Efficiency";
const QString CPUFrequencyControl::APPLY_TO_ALL_PERFORMANCE = "Performance";

CPUFrequencyControl::CPUFrequencyControl(DataProvider *dataProvider,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CPUFrequencyControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    m_cpuFreqData           = m_dataProvider->getDataMessage<legion::messages::CPUFrequency>(LenovoLegionDaemon::SysFsDataProviderCPUFrequency::dataType);;
    m_cpuTopologyData       = m_dataProvider->getDataMessage<legion::messages::CPUTopology>(LenovoLegionDaemon::SysFsDataProviderCPUTopology::dataType);


    ui->comboBoxApplyTo->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBoxApplyTo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);


    if(m_cpuFreqData.cpus().empty() ||
       m_cpuTopologyData.active_cpus_core().empty()
      )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "CPU Frequency Control data not available");
    }

    /*
     * CPU Control render
     */

    forAllActiveCpuPerformanceCores([this](const int i)
                              {
        ui->gridLayout_CPUFreqControl->addWidget(new ThreadFrequencyControl(QString("P-Thread: %1, CoreId: %2, DieId: %3").arg(i).arg(m_cpuFreqData.cpus().at(i).core_id()).arg(m_cpuFreqData.cpus().at(i).die_id()),
                                                                                                         m_cpuFreqData.cpus().at(i).min_freq()    / 1000,
                                                                                                         m_cpuFreqData.cpus().at(i).max_freq()    / 1000,
                                                                                                         m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000,
                                                                                                         m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000,
                                                                                                         "#3D2B1F",
                                                                                                         this
                                                                                                         )
                                                                                      ,
                                                                                      i % 8,i/8);
                                return true;
                              });

    /*
     * CPU Control render
     */
    forAllActiveCpuEfficientCores([this](const int i){
        ui->gridLayout_CPUFreqControl->addWidget(new ThreadFrequencyControl(QString("E-Thread: %1, CoreId: %2, DieId: %3").arg(i).arg(m_cpuFreqData.cpus().at(i).core_id()).arg(m_cpuFreqData.cpus().at(i).die_id()),
                                                                             m_cpuFreqData.cpus().at(i).min_freq()    / 1000,
                                                                             m_cpuFreqData.cpus().at(i).max_freq()    / 1000,
                                                                             m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000,
                                                                             m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000,
                                                                             "#393939",
                                                                             this
                                                                             )
                                                          ,
                                                          i % 8,i/8);
        return true;
    });


    ui->horizontalSlider_CPUFreqControlMin->blockSignals(true);
    ui->horizontalSlider_CPUFreqControlMax->blockSignals(true);
    forAllActiveCPUThreads([this](const int i)
    {
        ui->horizontalSlider_CPUFreqControlMin->setMinimum(m_cpuFreqData.cpus().at(i).min_freq() / 1000);
        ui->horizontalSlider_CPUFreqControlMin->setMaximum(m_cpuFreqData.cpus().at(i).max_freq() / 1000);

        ui->horizontalSlider_CPUFreqControlMax->setMinimum(m_cpuFreqData.cpus().at(i).min_freq() / 1000);
        ui->horizontalSlider_CPUFreqControlMax->setMaximum(m_cpuFreqData.cpus().at(i).max_freq() / 1000);

        ui->horizontalSlider_CPUFreqControlMin->setValue(m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
        ui->horizontalSlider_CPUFreqControlMax->setValue(m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);

        ui->horizontalSlider_CPUFreqControlMin->setPageStep(1);
        ui->horizontalSlider_CPUFreqControlMax->setPageStep(1);
        return false;
    });
    forAllActiveCPUThreads([this](const int i)
    {
        ui->horizontalSlider_CPUFreqControlMin->setMinimum(std::min(ui->horizontalSlider_CPUFreqControlMin->minimum(),static_cast<int>(m_cpuFreqData.cpus().at(i).min_freq() / 1000)));
        ui->horizontalSlider_CPUFreqControlMin->setMaximum(std::max(ui->horizontalSlider_CPUFreqControlMin->maximum(),static_cast<int>(m_cpuFreqData.cpus().at(i).max_freq() / 1000)));

        ui->horizontalSlider_CPUFreqControlMax->setMinimum(std::min(ui->horizontalSlider_CPUFreqControlMax->minimum(),static_cast<int>(m_cpuFreqData.cpus().at(i).min_freq() / 1000)));
        ui->horizontalSlider_CPUFreqControlMax->setMaximum(std::max(ui->horizontalSlider_CPUFreqControlMax->maximum(),static_cast<int>(m_cpuFreqData.cpus().at(i).max_freq() / 1000)));
        return true;
    });
    ui->horizontalSlider_CPUFreqControlMin->blockSignals(false);
    ui->horizontalSlider_CPUFreqControlMax->blockSignals(false);

    ui->comboBoxApplyTo->addItem(APPLY_TO_ALL);
    if(!m_cpuTopologyData.active_cpus_core().empty() &&
       !m_cpuTopologyData.active_cpus_atom().empty())
    {
        ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_PERFORMANCE);
        ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_EFFICIENT);
    }

    renderData();
}

void CPUFrequencyControl::on_horizontalSlider_CPUFreqControlMin_valueChanged(int value)
{
    ui->horizontalSlider_CPUFreqControlMax->setValue(ui->horizontalSlider_CPUFreqControlMax->value() < value ? value : ui->horizontalSlider_CPUFreqControlMax->value());

    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL ||
       ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllActiveCpuPerformanceCores([this,value](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setMinFreq(value,m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
            return true;
        });
    }

    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL ||
       ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_EFFICIENT)
    {
        forAllActiveCpuEfficientCores([this,value](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setMinFreq(value,m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
            return true;
        });
    }
}

void CPUFrequencyControl::on_horizontalSlider_CPUFreqControlMax_valueChanged(int value)
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->value() > value ? value : ui->horizontalSlider_CPUFreqControlMin->value());

    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL ||
        ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllActiveCpuPerformanceCores([this,value](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setMaxFreq(value,m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);
            return true;
        });
    }

    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL ||
        ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_EFFICIENT)
    {
        forAllActiveCpuEfficientCores([this,value](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setMaxFreq(value,m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);
            return true;
        });
    }

}


void CPUFrequencyControl::on_pushButton_CPUFreqControlCancel_clicked()
{
    renderData();
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlApply_clicked()
{

    forAllActiveCpuPerformanceCores([this](const int i){
        m_cpuFreqData.mutable_cpus()->at(i).set_scaling_min_freq(dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->getMinFreq() * 1000);
        m_cpuFreqData.mutable_cpus()->at(i).set_scaling_max_freq(dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->getMaxFreq() * 1000);
        return true;
    });

    forAllActiveCpuEfficientCores([this](const int i){
        m_cpuFreqData.mutable_cpus()->at(i).set_scaling_min_freq(dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->getMinFreq() * 1000);
        m_cpuFreqData.mutable_cpus()->at(i).set_scaling_max_freq(dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->getMaxFreq() * 1000);
        return true;
    });

    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUFrequency::dataType,m_cpuFreqData);

    renderData();

    emit widgetEvent(LenovoLegionGui::WidgetMessage(LenovoLegionGui::WidgetMessage::Widget::CPU_FREQUENCY_CONTROL,LenovoLegionGui::WidgetMessage::Message::CPU_FREQ_CONTROL_APPLY));
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin2500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(2500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(2500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl25002500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(2500 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(2500);
    ui->horizontalSlider_CPUFreqControlMax->setValue(2500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(2500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin3000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(3000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3000);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl30003000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(3000 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(3000);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3000);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin3500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(3500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl35003500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(3500 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(3500);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(3500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin4000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(4000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4000);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl40004000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(4000 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(4000);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4000);
}


void CPUFrequencyControl::on_pushButton_CPUFreqControlDefault_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(ui->horizontalSlider_CPUFreqControlMax->maximum() - 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(ui->horizontalSlider_CPUFreqControlMax->maximum());
}
void CPUFrequencyControl::forAllActiveCpuPerformanceCores(const std::function<bool (const int)> &func)
{
    if(m_cpuTopologyData.active_cpus_core().size() == 0)
    {
        Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopologyData.active_cpus());
        return;
    }

    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopologyData.active_cpus_core());
}

void CPUFrequencyControl::forAllActiveCpuEfficientCores(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopologyData.active_cpus_atom());
}

void CPUFrequencyControl::forAllActiveCPUThreads(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopologyData.active_cpus());
}

void CPUFrequencyControl::renderData()
{
    if(m_cpuFreqData.cpus().empty())
    {
        emit widgetEvent( LenovoLegionGui::WidgetMessage {
            .m_widget  = LenovoLegionGui::WidgetMessage::Widget::CPU_FREQUENCY_CONTROL,
            .m_message = LenovoLegionGui::WidgetMessage::Message::CPU_FREQ_CONTROL_NOT_AVAILABLE
        });

        return;
    }

    /*
     * Render CPU Frequency Performance
     */
    forAllActiveCpuPerformanceCores([this](const int i){
        dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i%8,i/8)->widget())->setMinFreq(m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000,m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
        dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i%8,i/8)->widget())->setMaxFreq(m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000,m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);
        return true;
    });

    /*
     * Render CPU Frequency Efficiency
     */
    forAllActiveCpuEfficientCores([this](const int i){
        dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i%8,i/8)->widget())->setMinFreq(m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000,m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
        dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i%8,i/8)->widget())->setMaxFreq(m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000,m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);
        return true;
    });


    ui->horizontalSlider_CPUFreqControlMin->blockSignals(true);
    ui->horizontalSlider_CPUFreqControlMax->blockSignals(true);
    forAllActiveCPUThreads([this](const int i)
    {
        ui->horizontalSlider_CPUFreqControlMin->setValue(m_cpuFreqData.cpus().at(i).scaling_min_freq() / 1000);
        ui->horizontalSlider_CPUFreqControlMax->setValue(m_cpuFreqData.cpus().at(i).scaling_max_freq() / 1000);
        return false;
    });

    ui->horizontalSlider_CPUFreqControlMin->blockSignals(false);
    ui->horizontalSlider_CPUFreqControlMax->blockSignals(false);
}


CPUFrequencyControl::~CPUFrequencyControl()
{
    delete ui;
}

void CPUFrequencyControl::refresh()
{
    renderData();
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin4500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(4500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl45004500_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(4500 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(4500);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4500 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(4500);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControlMin5000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum() + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(ui->horizontalSlider_CPUFreqControlMin->minimum());
    ui->horizontalSlider_CPUFreqControlMax->setValue(5000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(5000);
}

void CPUFrequencyControl::on_pushButton_CPUFreqControl50005000_clicked()
{
    ui->horizontalSlider_CPUFreqControlMin->setValue(5000 + 1);
    ui->horizontalSlider_CPUFreqControlMin->setValue(5000);
    ui->horizontalSlider_CPUFreqControlMax->setValue(5000 + 1);
    ui->horizontalSlider_CPUFreqControlMax->setValue(5000);
}

void CPUFrequencyControl::on_comboBoxApplyTo_currentTextChanged(const QString &arg1)
{
    if(arg1 == APPLY_TO_ALL)
    {
        forAllActiveCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllActiveCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllActiveCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllActiveCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_EFFICIENT)
    {
        forAllActiveCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });

        forAllActiveCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadFrequencyControl*>(ui->gridLayout_CPUFreqControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });
    }
}







}
