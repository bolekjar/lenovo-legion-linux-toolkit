// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "WidgetMessage.h"


#include "../LenovoLegion-PrepareBuild/HWMonitoring.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUTopology.pb.h"
#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"
#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"

#include <QWidget>

#include <chrono>

namespace Ui {
class HWMonitoring;
}

namespace LenovoLegionGui {

class DataProvider;
class CPUFrequency;
class GPUDetails;
class GPUDetails;
class CPUDetails;

class HWMonitoring : public QWidget
{
    Q_OBJECT

private:

    static constexpr int TIMER_EVENT_IN_MS = 500;

public:
    explicit HWMonitoring(DataProvider *dataProvider,QWidget *parent = nullptr);


    virtual ~HWMonitoring();

    virtual void timerEvent(QTimerEvent *event) override;

    void refresh();

private slots:
    void on_groupBox_CPU_Per_Thr_clicked(bool checked);
    void freqInfoByCoreClosed();
    void gpuDetailsClosed();

    void on_groupBox_Power_clicked(bool checked);

private:
    void forAllCpuPerformanceCores(const std::function<bool(const int index)> &func);
    void forAllCpuEfficientCores(const std::function<bool(const int index)> &func);

signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);
private:

    Ui::HWMonitoring *ui;

    legion::messages::HardwareMonitor                         m_hwMonitoringData;
    legion::messages::CPUTopology                             m_cpuTopology;
    legion::messages::NvidiaNvml                              m_nvidiaNvmlData;
    legion::messages::CPUInfo                                 m_cpuInfoData;

    DataProvider                *m_dataProvider;
    CPUFrequency                *m_windowFreqInfoByCore;
    GPUDetails                  *m_windowGPUDetails;

    std::chrono::steady_clock::time_point    m_lastRefreshTime;

    int m_timerId = -1;
};

}
