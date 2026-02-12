// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "CPUControl.h"
#include "ui_CPUControl.h"

#include "DataProvider.h"

#include "ThreadControl.h"
#include "Utils.h"

#include "../LenovoLegion-Daemon/SysFsDataProviderCPUOptions.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUTopology.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUSMT.h"

#include <QAbstractItemView>

namespace LenovoLegionGui {


QString const CPUControl::NAME      =   "CPU Control";


static legion::messages::CPUOptions::CPUX CPUOption(const char* gouvernor,bool online)
{
    legion::messages::CPUOptions::CPUX cpux;

    cpux.set_governor(gouvernor);
    cpux.set_cpu_online(online);

    return cpux;
}


const QMap<QString,legion::messages::CPUOptions::CPUX>CPUControl::CPU_CONTROL_PRESETS = {
    {CPUControl::PERFORMANCE.data(),CPUOption("performance",true)},
    {CPUControl::POWER_SAVE.data(),CPUOption("powersave",true)} ,
    {CPUControl::ONDEMAND.data(),CPUOption("performance",true)},
    {CPUControl::OFF.data(),CPUOption("powersave",false)}
};


const QString CPUControl::APPLY_TO_ALL              = "ALL";
const QString CPUControl::APPLY_TO_ALL_ENABLED      = "Enabled";
const QString CPUControl::APPLY_TO_ALL_EFFICIENT    = "Efficiency";
const QString CPUControl::APPLY_TO_ALL_PERFORMANCE  = "Performance";
const QString CPUControl::APPLY_TO_ALL_DISABLED     = "Disabled";



static legion::messages::CPUSMT getSMTData(const char* value,bool active)
{
    legion::messages::CPUSMT smt;

    smt.set_active(active);
    smt.set_control(value);

    return smt;
}


const legion::messages::CPUSMT CPUControl::SMT_ON_DATA =  getSMTData("on",true);
const legion::messages::CPUSMT CPUControl::SMT_OFF_DATA = getSMTData("off",false);


legion::messages::CPUOptions::CPUX CPUControl::getCpuControlPreset(const QString &presetName, const legion::messages::CPUOptions::CPUX &dataInfo)
{
    auto availableGovernors = QString(dataInfo.available_governors().data()).trimmed().split(' ');

    if(CPU_CONTROL_PRESETS.contains(presetName))
    {
        legion::messages::CPUOptions::CPUX cpuControl = CPU_CONTROL_PRESETS.value(presetName);

        if(availableGovernors.contains(cpuControl.governor().data()))
        {
            return cpuControl;
        }

        cpuControl.set_governor(dataInfo.governor());

        return cpuControl;
    }

    return dataInfo;
}

CPUControl::CPUControl(DataProvider *dataProvider, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CPUControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    m_cpuInfoData = m_dataProvider->getDataMessage<legion::messages::CPUOptions>(LenovoLegionDaemon::SysFsDataProviderCPUOptions::dataType);
    m_cpuTopology = m_dataProvider->getDataMessage<legion::messages::CPUTopology>(LenovoLegionDaemon::SysFsDataProviderCPUTopology::dataType);
    m_cpuSMTControlData = m_dataProvider->getDataMessage<legion::messages::CPUSMT>(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType);


    if(m_cpuInfoData.cpus().empty()        ||
       m_cpuTopology.active_cpus().empty()
       )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "CPU Control data not available");
    }


    ui->comboBox_CPUGovernor->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_CPUGovernor->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBoxApplyTo->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBoxApplyTo->view()->window()->setAttribute(Qt::WA_TranslucentBackground);


    /*
     * CPU Control Performance GUI elements
     */
    class
    {
    public:
        const QString& getColor(quint32 coreId)
        {
            if(coreId != m_coreId)
            {
                m_curentColor++;

                if(m_curentColor == m_cpuThreadsColors.end())
                {
                    m_curentColor = m_cpuThreadsColors.begin();
                }

                m_coreId = coreId;
            }

            return *m_curentColor;
        }
    private:

        std::list<QString>                 m_cpuThreadsColors  = { "white", "black"};
        std::list<QString>::const_iterator m_curentColor       = m_cpuThreadsColors.begin();

        quint32 m_coreId = 0;
    } l_ColorGenerator;



    /*
     * CPU Control Performance GUI elements
     */
    forAllCpuPerformanceCores([this,&l_ColorGenerator](const int i){
        ui->gridLayout_CPUControl->addWidget(new ThreadControl(QString("P-Thread: %1, CoreId: %2, DieId: %3").
                                                               arg(i).
                                                               arg(m_cpuInfoData.cpus().at(i).cpu_core_id()).
                                                               arg(m_cpuInfoData.cpus().at(i).die_id()),i,m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).governor().data(),m_cpuInfoData.cpus().at(i).cpu_core_id(),l_ColorGenerator.getColor(m_cpuInfoData.cpus().at(i).cpu_core_id()),"#3D2B1F",this),i % 8,i/8);
        return true;
    });

    /*
     * CPU Control Efficiency GUI elements
     */
    forAllCpuEfficientCores([this,&l_ColorGenerator](const int i){
        ui->gridLayout_CPUControl->addWidget(new ThreadControl(QString("E-Thread: %1, CoreId: %2, DieId: %3").
                                                               arg(i).
                                                               arg(m_cpuInfoData.cpus().at(i).cpu_core_id()).
                                                               arg(m_cpuInfoData.cpus().at(i).die_id()),i,m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).governor().data(),m_cpuInfoData.cpus().at(i).cpu_core_id(),l_ColorGenerator.getColor(m_cpuInfoData.cpus().at(i).cpu_core_id()),"#393939",this),i % 8,i/8);
        return true;
    });

    /*
     * CPU Disabled Control GUI elements
     */
    forAllCpuDisabledCores([this,&l_ColorGenerator](const int i){
        ui->gridLayout_CPUControl->addWidget(new ThreadControl(QString("D-Thread: %1, CoreId: %2, DieId: %3").
                                                               arg(i).
                                                               arg(m_cpuInfoData.cpus().at(i).cpu_core_id()).
                                                               arg(m_cpuInfoData.cpus().at(i).die_id()),i,m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).governor().data(),m_cpuInfoData.cpus().at(i).cpu_core_id(),l_ColorGenerator.getColor(m_cpuInfoData.cpus().at(i).cpu_core_id()),"transparent",this),i % 8,i/8);
        return true;
    });

    if(m_cpuInfoData.cpus().size() > 0)
    {
        ui->comboBox_CPUGovernor->addItems(QString(m_cpuInfoData.cpus().at(0).available_governors().data()).trimmed().split(' '));
        ui->comboBox_CPUGovernor->setCurrentText(QString(m_cpuInfoData.cpus().at(0).governor().data()).trimmed());
    }

    renderData();

    ui->comboBoxApplyTo->addItem(APPLY_TO_ALL);
    ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_ENABLED);
    ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_DISABLED);

    if(!m_cpuTopology.active_cpus_core().empty() && !m_cpuTopology.active_cpus_atom().empty())
    {
        ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_PERFORMANCE);
        ui->comboBoxApplyTo->addItem(APPLY_TO_ALL_EFFICIENT);
    }

}

CPUControl::~CPUControl()
{
    delete ui;
}

void CPUControl::refresh()
{
    renderData();
}

void CPUControl::on_pushButton_CPUControlEnableAll_clicked()
{

    if( ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_ENABLED)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_DISABLED)
    {
        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_EFFICIENT)
    {
        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(true,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
}

void CPUControl::on_pushButton_CPUControlDisableAll_clicked()
{
    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_ENABLED)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_DISABLED)
    {
        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_EFFICIENT)
    {
        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(false,m_cpuInfoData.cpus().at(i).cpu_online());
            return true;
        });
    }
}

void CPUControl::on_comboBox_CPUGovernor_currentTextChanged(const QString &arg1)
{
    if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL)
    {
        forAllCpuPerformanceCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });

        forAllCpuEfficientCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_ENABLED)
    {
        forAllCpuPerformanceCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });

        forAllCpuEfficientCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllCpuPerformanceCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });
    }
    else if(ui->comboBoxApplyTo->currentText() == APPLY_TO_ALL_EFFICIENT)
    {
        forAllCpuEfficientCores([this,arg1](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(arg1,QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
            return true;
        });
    }
}

void CPUControl::on_pushButton_CPUControlCancel_clicked()
{
    renderData();
}

void CPUControl::on_pushButton_CPUControlApply_clicked()
{
    legion::messages::CPUOptions data = m_cpuInfoData;


    /*
     * CPU Control Performance GUI elements
     */
    forAllCpuPerformanceCores([this,&data](const int i){
        data.mutable_cpus()->at(i).set_cpu_online(dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->getCPUEnabled());
        data.mutable_cpus()->at(i).set_governor(dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->getScalingGovernor().toStdString().c_str());
        return true;
    });


    /*
     * CPU Control Efficiency GUI elements
     */
    forAllCpuEfficientCores([this,&data](const int i){
        data.mutable_cpus()->at(i).set_cpu_online(dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->getCPUEnabled());
        data.mutable_cpus()->at(i).set_governor(dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->getScalingGovernor().toStdString().c_str());
        return true;
    });

    /*
     * CPU Disabled Control GUI elements
     */
    forAllCpuDisabledCores([this,&data](const int i){
        data.mutable_cpus()->at(i).set_cpu_online(dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->getCPUEnabled());
        return true;
    });

    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUOptions::dataType,data);

    emit widgetEvent(LenovoLegionGui::WidgetMessage{LenovoLegionGui::WidgetMessage::Widget::CPU_CONTROL,LenovoLegionGui::WidgetMessage::Message::CPU_CONTROL_CHANGED});
}

void CPUControl::forAllCpuPerformanceCores(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopology.active_cpus_core());
}

void CPUControl::forAllCpuEfficientCores(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopology.active_cpus_atom());
}

void CPUControl::forAllCpuDisabledCores(const std::function<bool (const int)> &func)
{
    for (int i = 0;i < m_cpuInfoData.cpus().size();++i) {
        if(!m_cpuInfoData.cpus().at(i).cpu_online()) {
            if(!func(i))
            {
                return;
            };
        }
    }

}

void CPUControl::renderData()
{
    if( m_cpuInfoData.cpus().empty()             ||
        m_cpuTopology.active_cpus().empty()      ||
        !m_cpuSMTControlData.has_active()        ||
        !m_cpuSMTControlData.has_control()
      )
    {
        emit widgetEvent( LenovoLegionGui::WidgetMessage {
            .m_widget  = LenovoLegionGui::WidgetMessage::Widget::CPU_CONTROL,
            .m_message = LenovoLegionGui::WidgetMessage::Message::CPU_CONTROL_NOT_AVAILABLE
        });

        return;
    }

    /*
     * CPU Control Performance GUI elements
     */
    forAllCpuPerformanceCores([this](const int i){
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).cpu_online());
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed(),QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
        return true;
    });

    /*
     * CPU Control Efficiency GUI elements
     */
    forAllCpuEfficientCores([this](const int i){
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).cpu_online());
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed(),QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
        return true;
    });

    /*
     * CPU Disabled Control GUI elements
     */
    forAllCpuDisabledCores([this](const int i){
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setCPUEnabled(m_cpuInfoData.cpus().at(i).cpu_online(),m_cpuInfoData.cpus().at(i).cpu_online());
        dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setScalingGovernor(QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed(),QString(m_cpuInfoData.cpus().at(i).governor().data()).trimmed());
        return true;
    });

    if(m_cpuInfoData.cpus().size() > 0)
    {
        ui->comboBox_CPUGovernor->setCurrentText(QString(m_cpuInfoData.cpus().at(0).governor().data()).trimmed());
    }

    ui->checkBox_DisableSMP->blockSignals(true);
    ui->checkBox_DisableSMP->setEnabled(false);
    ui->checkBox_DisableSMP->setVisible(false);
    if(QString(m_cpuSMTControlData.control().data()).trimmed() == SMT_OFF_DATA.control().data() ||
       QString(m_cpuSMTControlData.control().data()).trimmed() == SMT_ON_DATA.control().data()
      )
    {
        if(m_cpuSMTControlData.active())
        {
            if(QString(m_cpuSMTControlData.control().data()).trimmed() == SMT_OFF_DATA.control().data())
            {
                m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType,SMT_ON_DATA);
            }

            ui->checkBox_DisableSMP->setChecked(false);
        }
        else
        {
            if(QString(m_cpuSMTControlData.control().data()).trimmed() == SMT_ON_DATA.control().data())
            {
                m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType,SMT_OFF_DATA);
            }

            ui->checkBox_DisableSMP->setChecked(true);
        }

        ui->checkBox_DisableSMP->setEnabled(true);
        ui->checkBox_DisableSMP->blockSignals(false);
        ui->checkBox_DisableSMP->setVisible(true);
    }
}


void CPUControl::on_checkBox_DisableSMP_checkStateChanged(const Qt::CheckState &arg1)
{
    if(arg1 == Qt::CheckState::Checked)
    {
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType,SMT_OFF_DATA);
    }
    else
    {
        m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderCPUSMT::dataType,SMT_ON_DATA);
    }


    emit widgetEvent(LenovoLegionGui::WidgetMessage{LenovoLegionGui::WidgetMessage::Widget::CPU_CONTROL,LenovoLegionGui::WidgetMessage::Message::CPU_CONTROL_CHANGED});
}

void CPUControl::on_comboBoxApplyTo_currentTextChanged(const QString &arg1)
{
    if(arg1 == APPLY_TO_ALL)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_ENABLED)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_DISABLED)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_PERFORMANCE)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });
    }
    else if(arg1 == APPLY_TO_ALL_EFFICIENT)
    {
        forAllCpuPerformanceCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });

        forAllCpuEfficientCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(true);
            return true;
        });

        forAllCpuDisabledCores([this](const int i){
            dynamic_cast<ThreadControl*>(ui->gridLayout_CPUControl->itemAtPosition(i % 8,i/8)->widget())->setVisible(false);
            return true;
        });
    }
}



}
