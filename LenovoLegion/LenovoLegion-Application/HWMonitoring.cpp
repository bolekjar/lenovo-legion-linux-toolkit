// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "HWMonitoring.h"
#include "ui_HWMonitoring.h"

#include "GPUDetails.h"
#include "Utils.h"
#include "DataProvider.h"

#include <Core/LoggerHolder.h>


#include "../LenovoLegion-Daemon/SysFsDataProviderHWMon.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUTopology.h"
#include "../LenovoLegion-Daemon/DataProviderNvidiaNvml.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUInfo.h"


#include "CPUFrequency.h"
#include "ThreadFrequency.h"


namespace LenovoLegionGui {

HWMonitoring::HWMonitoring(DataProvider *dataProvider,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HWMonitoring)
    , m_dataProvider(dataProvider)
    , m_windowFreqInfoByCore(new CPUFrequency(nullptr))
    , m_windowGPUDetails(new GPUDetails(nullptr))
{
    ui->setupUi(this);

    m_latestHWMonitoringData = m_hwMonitoringData = m_dataProvider->getDataMessage<legion::messages::HardwareMonitor>(LenovoLegionDaemon::SysFsDataProviderHWMon::dataType);
    m_cpuTopology      = m_dataProvider->getDataMessage<legion::messages::CPUTopology>(LenovoLegionDaemon::SysFsDataProviderCPUTopology::dataType);
    m_latestNvidiaNvmlData = m_nvidiaNvmlData   = m_dataProvider->getDataMessage<legion::messages::NvidiaNvml>(LenovoLegionDaemon::DataProviderNvidiaNvml::dataType);
    m_cpuInfoData      = m_dataProvider->getDataMessage<legion::messages::CPUInfo>(LenovoLegionDaemon::SysFsDataProviderCPUInfo::dataType);

    /*
     * CPU Freq Info Performance GUI elements
     */
    forAllCpuPerformanceCores([this](const int i){
        m_windowFreqInfoByCore->addPerformanceWidget(i,new ThreadFrequency(QString("P-Core: ") + QString::number(i) ,m_hwMonitoringData.cpux_freq().at(i).cpu_info_min_freq()/1000,m_hwMonitoringData.cpux_freq().at(i).cpu_info_max_freq()/1000,this));
        return true;
    });

    /*
     * CPU Freq Info Efficiency GUI elements
     */
    forAllCpuEfficientCores([this](const int i){
        m_windowFreqInfoByCore->addEfficiencyWidget(i,new ThreadFrequency(QString("E-Core: ") + QString::number(i),m_hwMonitoringData.cpux_freq().at(i).cpu_info_min_freq()/1000,m_hwMonitoringData.cpux_freq().at(i).cpu_info_max_freq()/1000,this));
        return true;
    });

    m_windowGPUDetails->init(m_nvidiaNvmlData);

    //ui->groupBox_CPU->setTitle(QString("CPU (%1), GPU (%2)").arg(m_cpuInfoData.cpu_model()).arg(m_nvidiaNvmlData.name()));
    ui->widget_CPUPower->init("CPU Power [%1 W]",0,250,0,250);

    quint32 pMin = std::numeric_limits<quint32>::max();
    quint32 pMax = std::numeric_limits<quint32>::min();
    quint32 pScaleMin = std::numeric_limits<quint32>::max();
    quint32 pScaleMax = std::numeric_limits<quint32>::min();
    forAllCpuPerformanceCores([&pMin,&pMax,&pScaleMin,&pScaleMax,this](const int i){
        pMin = std::min(m_hwMonitoringData.cpux_freq().at(i).cpu_info_min_freq()/1000,pMin);
        pMax = std::max(m_hwMonitoringData.cpux_freq().at(i).cpu_info_max_freq()/1000,pMax);
        pScaleMin = std::min(m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_min_freq()/1000,pScaleMin);
        pScaleMax = std::max(m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_max_freq()/1000,pScaleMax);
        return true;
    });
    ui->widget_PCoresAvgFreq->init("P-Core [%1 MHz]",pMin,pMax,pScaleMin,pScaleMax);

    quint32 eMin = std::numeric_limits<quint32>::max();
    quint32 eMax = std::numeric_limits<quint32>::min();
    quint32 eScaleMin = std::numeric_limits<quint32>::max();
    quint32 eScaleMax = std::numeric_limits<quint32>::min();
    forAllCpuEfficientCores([&eMin,&eMax,&eScaleMin,&eScaleMax,this](const int i){
        eMin = std::min(m_hwMonitoringData.cpux_freq().at(i).cpu_info_min_freq()/1000,eMin);
        eMax = std::max(m_hwMonitoringData.cpux_freq().at(i).cpu_info_max_freq()/1000,eMax);
        eScaleMin = std::min(m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_min_freq()/1000,eScaleMin);
        eScaleMax = std::max(m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_max_freq()/1000,eScaleMax);
        return true;
    });

    if(eMin == std::numeric_limits<quint32>::max()||
       eMax == std::numeric_limits<quint32>::min())
    {
        ui->widget_ECoresAvgFreq->init("E-Core [%1 MHz]",0,0,0,0);
        ui->widget_CPUAvgFreq->init("Avg [%1 MHz]",pMin,pMax,pScaleMin,pScaleMax);
    }
    else
    {
        ui->widget_ECoresAvgFreq->init("E-Core [%1 MHz]",eMin,eMax,eScaleMin,eScaleMax);
        ui->widget_CPUAvgFreq->init("Avg [%1 MHz]",std::min(pMin,eMin),std::max(pMax,eMax),std::min(pScaleMin,eScaleMin),std::max(pScaleMax,eScaleMax));
    }

    m_windowFreqInfoByCore->init(m_cpuInfoData,m_cpuTopology,std::min(pScaleMin,eScaleMin),std::max(pScaleMax,eScaleMax));

    ui->widget_GPUPower->init("GPU Power [%1 W]",0,m_nvidiaNvmlData.hardware_monitor().power().max_value() / 1000,0,m_nvidiaNvmlData.hardware_monitor().power().max_value() / 1000);

    /*
     * Fans
     */
    for (int i = 0; i < m_hwMonitoringData.legion().fans_size(); ++i)
    {
        auto fanWidget = new HWMonitor(this);
        fanWidget->init(m_hwMonitoringData.legion().fans().at(i).fan_label().data() + QString(" [%1 RPM]"),m_hwMonitoringData.legion().fans().at(i).fan_speed_min(),m_hwMonitoringData.legion().fans().at(i).fan_speed_max() + 100,m_hwMonitoringData.legion().fans().at(i).fan_speed_min(),m_hwMonitoringData.legion().fans().at(i).fan_speed_max() + 100);
        ui->horizontalLayout_Fans->addWidget(fanWidget);
    }

    /*
     * Temps
     */
    for (int i = 0; i < m_hwMonitoringData.legion().temps_size(); ++i)
    {
        auto tempWidget = new HWMonitor(this);
        tempWidget->init(m_hwMonitoringData.legion().temps().at(i).temp_label().data() + QString(" [%1 °C]"),30,110,30,110);
        ui->horizontalLayout_Temp->addWidget(tempWidget);
    }


    connect(m_windowFreqInfoByCore,&CPUFrequency::closed,this,&HWMonitoring::freqInfoByCoreClosed);
    connect(m_windowGPUDetails,&GPUDetails::closed,this,&HWMonitoring::gpuDetailsClosed);


    m_timerId = startTimer(TIMER_EVENT_IN_MS);
}

void HWMonitoring::refresh()
{
    QtConcurrent::run([this](){
        return this->prepareData();
    }).then([this](UIUpdatePackage package){
        QTimer::singleShot(0, this, [this, package]() {
            try {
                for (size_t i = 0; i < package.m_tempValues.size(); ++i)
                {
                    dynamic_cast<HWMonitor*>(ui->horizontalLayout_Temp->itemAt(i)->widget())->refresh(package.m_tempValues.at(i),
                                                                                                       0,
                                                                                                       10,
                                                                                                       ' ',
                                                                                                       package.m_tempTexts.at(i));
                }

                ui->widget_CPUPower->refresh(package.m_cpuPowerValue,
                                             0,10,' ',
                                             package.m_cpuPowerText);

                ui->widget_GPUPower->refresh(package.m_gpuPowerValue,0,10,' ',package.m_gpuPowerText);


                for (size_t i = 0; i < package.m_fanSpeeds.size() && i < static_cast<size_t>(ui->horizontalLayout_Fans->count()); ++i)
                {
                    dynamic_cast<HWMonitor*>(ui->horizontalLayout_Fans->itemAt(i)->widget())->refresh(package.m_fanSpeeds.at(i),
                                                                                                       0,
                                                                                                       10,
                                                                                                       ' ',
                                                                                                       package.m_fanTexts.at(i));
                }

                if(m_windowFreqInfoByCore->isVisible())
                {
                    for(const auto &coreData : package.m_coresData)
                    {
                        QWidget* w = coreData.m_isPerformance ?
                                         m_windowFreqInfoByCore->getPerforamnceWidget(coreData.m_index) :
                                         m_windowFreqInfoByCore->getEfficiencyWidget(coreData.m_index);


                        if (auto* tfWidget = dynamic_cast<ThreadFrequency*>(w))
                        {
                            tfWidget->setBaseFreq(coreData.m_base);
                            tfWidget->setMinFreq(coreData.m_min);
                            tfWidget->setMaxFreq(coreData.m_max);
                            tfWidget->setScalingMinFreq(coreData.m_scalingMin);
                            tfWidget->setScalingMaxFreq(coreData.m_scalingMax);
                            tfWidget->setScalingCurFreq(coreData.m_scalingCur);
                            tfWidget->refreshToolTip();
                            tfWidget->refresh();
                        }
                    }
                }

                if(m_windowGPUDetails->isVisible())
                {
                    /*
                     * GPU Details
                     */
                    m_windowGPUDetails->refresh(m_latestNvidiaNvmlData);
                }

                ui->widget_CPUAvgFreq->refresh(package.m_averageFreq,4,10,QChar('0'),package.m_averageFreqText);
                ui->widget_PCoresAvgFreq->refresh(package.m_averageFreqPcores,4,10,QChar('0'),package.m_averageFreqPcoresText);
                ui->widget_ECoresAvgFreq->refresh(package.m_averageFreqEcores,4,10,QChar('0'),package.m_averageFreqEcoresText);

            } catch(ProtocolProcessor::exception_T &ex) {
                LOG_W(QString("HWMonitoring refresh error: ").append(ex.what()));
            }
        });
    });
}

HWMonitoring::~HWMonitoring()
{
    killTimer(m_timerId);

    delete m_windowFreqInfoByCore;
    delete m_windowGPUDetails;
    delete ui;
}

void HWMonitoring::timerEvent(QTimerEvent *)
{
    if(m_pendingRequests > 0)
    {
        LOG_W(QString("HWMonitoring timerEvent: pending requests > 0, skipping refresh"));
        return;
    }

    m_pendingRequests++;
    m_dataProvider->getDataMessageAsync<legion::messages::HardwareMonitor>(LenovoLegionDaemon::SysFsDataProviderHWMon::dataType).then([this](const legion::messages::HardwareMonitor &data){
        m_hwMonitoringData = m_latestHWMonitoringData;
        m_latestHWMonitoringData = data;

        m_pendingRequests--;

        if(m_pendingRequests == 0)
        {
            QTimer::singleShot(0, this, [this]() {
                this->refresh();
            });
        }
    });

    m_pendingRequests++;
    m_dataProvider->getDataMessageAsync<legion::messages::NvidiaNvml>(LenovoLegionDaemon::DataProviderNvidiaNvml::dataType).then([this](const legion::messages::NvidiaNvml &data){
        m_nvidiaNvmlData = m_latestNvidiaNvmlData;
        m_latestNvidiaNvmlData = data;

        m_pendingRequests--;

        if(m_pendingRequests == 0)
        {
            QTimer::singleShot(0, this, [this]() {
                this->refresh();
            });
        }
    });
}

void HWMonitoring::forAllCpuPerformanceCores(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopology.active_cpus_core());
}

void HWMonitoring::forAllCpuEfficientCores(const std::function<bool (const int)> &func)
{
    Utils::ProtoBuf::forAllCpuTopologyRange(func,m_cpuTopology.active_cpus_atom());
}

HWMonitoring::UIUpdatePackage HWMonitoring::prepareData()
{
    UIUpdatePackage package;

    if(m_pendingRequests > 0)
    {
        LOG_W(QString("HWMonitoring prepareData: pending requests > 0, skipping refresh"));
        return package;
    }

    /*
     * Temps
     */
    package.m_tempValues.reserve(m_hwMonitoringData.legion().temps_size());
    package.m_tempTexts.reserve(m_hwMonitoringData.legion().temps_size());
    for (int i = 0; i < m_hwMonitoringData.legion().temps_size(); ++i)
    {
        package.m_tempValues.push_back(m_latestHWMonitoringData.legion().temps().at(i).temp_value() / 1000);
        package.m_tempTexts.push_back(QString("%1: %2 °C\n"\
                                            "%1 min: 30 °C\n"\
                                            "%1 max: 100 °C\n").arg(m_latestHWMonitoringData.legion().temps().at(i).temp_label().data()).arg(m_latestHWMonitoringData.legion().temps().at(i).temp_value() / 1000));
    }

    /*
     * Cpu Power
     */
    {
        quint64 diff = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_lastRefreshTime).count();

        if(diff < (TIMER_EVENT_IN_MS * 1200) && diff > (TIMER_EVENT_IN_MS * 800))
        {
            package.m_cpuPowerValue = (m_latestHWMonitoringData.intel_power().power_cap_cpu_energy() - m_hwMonitoringData.intel_power().power_cap_cpu_energy())/diff;
            package.m_cpuPowerText = QString("%1\n"
                                      "Power : %2 W\n"
                                      "Power min: 0 W\n"
                                      "Power max: 250 W\n").arg(m_cpuInfoData.cpu_model()).arg((m_latestHWMonitoringData.intel_power().power_cap_cpu_energy() - m_hwMonitoringData.intel_power().power_cap_cpu_energy())/diff);
        }
        else
        {
            package.m_cpuPowerValue = ui->widget_CPUPower->getValue();
            package.m_cpuPowerText = QString("%1\n"
                                      "Power : %2 W\n"
                                      "Power min: 0 W\n"
                                      "Power max: 250 W\n").arg(m_cpuInfoData.cpu_model()).arg(ui->widget_CPUPower->getValue());
        }
    }


    /*
     * GPU Power
     */
    package.m_gpuPowerValue = m_latestNvidiaNvmlData.hardware_monitor().power().value() / 1000;
    package.m_gpuPowerText  =  QString("%1\n"\
                                   "Power: %2 W\n"\
                                   "Power min: %3 W\n"\
                                   "Power max: %4 W").arg(m_latestNvidiaNvmlData.name()).arg(m_latestNvidiaNvmlData.hardware_monitor().power().value() / 1000)
                                    .arg(m_latestNvidiaNvmlData.hardware_monitor().power().min_value() / 1000)
                                    .arg(m_nvidiaNvmlData.hardware_monitor().power().max_value() / 1000);


    /*
     * Fans
     */
    package.m_fanSpeeds.reserve(m_hwMonitoringData.legion().fans_size());
    package.m_fanTexts.reserve(m_hwMonitoringData.legion().fans_size());
    for (int i = 0; i < m_hwMonitoringData.legion().fans_size() && i < ui->horizontalLayout_Fans->count(); ++i)
    {
        package.m_fanSpeeds.push_back(m_hwMonitoringData.legion().fans().at(i).fan_speed());
        package.m_fanTexts.push_back(QString("Fan %1 Speed: %2 RPM\n"\
                                           "Fan %1 Speed min: %3 RPM\n"\
                                           "Fan %1 Speed max: %4 RPM\n").arg(m_hwMonitoringData.legion().fans().at(i).fan_label())
                                                                       .arg(m_hwMonitoringData.legion().fans().at(i).fan_speed())
                                                                       .arg(m_hwMonitoringData.legion().fans().at(i).fan_speed_min())
                                                                       .arg(m_hwMonitoringData.legion().fans().at(i).fan_speed_max() + 100));
    }


    struct Stats {
        int averageFreq             = 0;
        int avareqePCoreFreq        = 0;
        int avareqeACoreFreq        = 0;
        int averageFreqCount        = 0;
        int avareqePCoreFreqCount   = 0;
        int avareqeACoreFreqCount   = 0;
    } l_stats;

    if(m_windowFreqInfoByCore->isVisible())
    {

        int maxAvailableFreqs = m_hwMonitoringData.cpux_freq_size();
        auto processCore = [&](int i, bool isParamPerformance) {
            if (i >= maxAvailableFreqs) return false;
            const auto& freqItem = m_hwMonitoringData.cpux_freq().at(i);

            UIUpdatePackage::CoreFreqData cData;
            cData.m_index = i;
            cData.m_isPerformance = isParamPerformance;
            cData.m_base = static_cast<double>(freqItem.cpu_base_freq()) / 1000.0;
            cData.m_min = static_cast<double>(freqItem.cpu_info_min_freq()) / 1000.0;
            cData.m_max = static_cast<double>(freqItem.cpu_info_max_freq()) / 1000.0;
            cData.m_scalingMin = static_cast<double>(freqItem.cpu_scaling_min_freq()) / 1000.0;
            cData.m_scalingMax = static_cast<double>(freqItem.cpu_scaling_max_freq()) / 1000.0;
            cData.m_scalingCur = static_cast<double>(freqItem.cpu_scaling_cur_freq()) / 1000.0;

            package.m_coresData.push_back(cData);

            if(isParamPerformance)
            {
                l_stats.avareqePCoreFreq += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.avareqePCoreFreqCount;
            }
            else
            {
                l_stats.avareqeACoreFreq += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.avareqeACoreFreqCount;
            }

            l_stats.averageFreq      += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.averageFreqCount;
            return true;
        };

        forAllCpuPerformanceCores([&](int i) { return processCore(i, true); });
        forAllCpuEfficientCores([&](int i) { return processCore(i, false); });
    }
    else
    {
        forAllCpuPerformanceCores([&](int i) {
            l_stats.avareqePCoreFreq += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.avareqePCoreFreqCount;
            l_stats.averageFreq      += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.averageFreqCount;
            return true;
        });
        forAllCpuEfficientCores([&](int i) {
            l_stats.avareqeACoreFreq += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.avareqeACoreFreqCount;
            l_stats.averageFreq      += m_hwMonitoringData.cpux_freq().at(i).cpu_scaling_cur_freq();++l_stats.averageFreqCount;
            return true;
        });
    }


    /*
         * Render CPU Frequency Avg
         */
    package.m_averageFreq = l_stats.averageFreqCount == 0 ? 0 : (l_stats.averageFreq/l_stats.averageFreqCount/1000);
    package.m_averageFreqPcores = l_stats.avareqePCoreFreqCount == 0 ? 0 : (l_stats.avareqePCoreFreq/l_stats.avareqePCoreFreqCount/1000);
    package.m_averageFreqEcores = l_stats.avareqeACoreFreqCount == 0 ? 0 : (l_stats.avareqeACoreFreq/l_stats.avareqeACoreFreqCount/1000);

    package.m_averageFreqText = QString("CPU Average Frequency: %1 MHz\n"\
                                           "CPU Min Frequency: %2 MHz\n"\
                                           "CPU Max Frequency: %3 MHz\n"\
                                           "CPU Scale Min Frequency: %4 MHz\n"\
                                           "CPU Scale Max Frequency: %5 MHz\n").arg(package.m_averageFreq).arg(ui->widget_CPUAvgFreq->getMinValue()).arg(ui->widget_CPUAvgFreq->getMaxValue()).arg(ui->widget_CPUAvgFreq->getScaleMin()).arg(ui->widget_CPUAvgFreq->getScaleMax());
    package.m_averageFreqPcoresText = QString("P-Core Average Frequency: %1 MHz\n"\
                                              "P-Core Min Frequency: %2 MHz\n"\
                                              "P-Core Max Frequency: %3 MHz\n"
                                              "P-Core Scale Min Frequency: %4 MHz\n"
                                              "P-Core Scale Maxn Frequency: %5 MHz\n").arg(package.m_averageFreqPcores).arg(ui->widget_PCoresAvgFreq->getMinValue()).arg(ui->widget_PCoresAvgFreq->getMaxValue()).arg((ui->widget_PCoresAvgFreq->getScaleMin())).arg(ui->widget_PCoresAvgFreq->getScaleMax());


    package.m_averageFreqEcoresText = QString("E-Core Average Frequency: %1 MHz\n"\
                                              "E-Core Min Frequency: %2 MHz\n"\
                                              "E-Core Max Frequency: %3 MHz\n"
                                              "E-Core Scale Min Frequency: %4 MHz\n"
                                              "E-Core Scale Max Frequency: %5 MHz\n").arg(package.m_averageFreqEcores).arg(ui->widget_ECoresAvgFreq->getMinValue()).arg(ui->widget_ECoresAvgFreq->getMaxValue()).arg(ui->widget_ECoresAvgFreq->getScaleMin()).arg(ui->widget_ECoresAvgFreq->getScaleMax());

    m_lastRefreshTime = std::chrono::steady_clock::now();


    return package;
}

void HWMonitoring::on_groupBox_CPU_Per_Thr_clicked(bool checked)
{
    if(m_windowFreqInfoByCore->isHidden() && checked)
    {
        m_windowFreqInfoByCore->show();
    }
    else
    {
        m_windowFreqInfoByCore->hide();
    }
}

void HWMonitoring::freqInfoByCoreClosed()
{
    ui->groupBox_CPU_Per_Thr->setChecked(false);
}

void HWMonitoring::gpuDetailsClosed()
{
    ui->groupBox_Power->setChecked(false);
}


void HWMonitoring::on_groupBox_Power_clicked(bool checked)
{
    if(m_windowGPUDetails->isHidden() && checked)
    {
        m_windowGPUDetails->show();
    }
    else
    {
        m_windowGPUDetails->hide();
    }
}


}
