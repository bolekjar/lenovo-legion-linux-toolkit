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
#include <QFuture>

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

    struct UIUpdatePackage {

        std::vector<uint32_t>   m_tempValues;
        std::vector<QString>    m_tempTexts;

        uint64_t m_cpuPowerValue = 0;
        QString  m_cpuPowerText;

        uint64_t m_gpuPowerValue = 0;
        QString  m_gpuPowerText;


        std::vector<uint32_t>   m_fanSpeeds;
        std::vector<QString>    m_fanTexts;

        struct CoreFreqData {
            int             m_index = 0;
            bool            m_isPerformance = true;
            uint32_t        m_base = 0;
            uint32_t        m_min = 0;
            uint32_t        m_max = 0;
            uint32_t        m_scalingMin = 0;
            uint32_t        m_scalingMax = 0;
            uint32_t        m_scalingCur = 0;
        };
        std::vector<CoreFreqData> m_coresData;

        uint32_t m_averageFreq = 0;
        uint32_t m_averageFreqPcores = 0;
        uint32_t m_averageFreqEcores = 0;

        QString m_averageFreqText;
        QString m_averageFreqPcoresText;
        QString m_averageFreqEcoresText;
    };

    enum class RefreshState {
        IDLE,
        FETCHING_DATA,
        PROCESSING_DATA,
        UPDATING_UI,
        DESTROYING
    };

public:
    explicit HWMonitoring(DataProvider *dataProvider,QWidget *parent = nullptr);


    virtual ~HWMonitoring();

    void refresh();

private:

    virtual void timerEvent(QTimerEvent *event) override;

private slots:
    void on_groupBox_CPU_Per_Thr_clicked(bool checked);
    void freqInfoByCoreClosed();
    void gpuDetailsClosed();

    void on_groupBox_Power_clicked(bool checked);

private:
    void forAllCpuPerformanceCores(const std::function<bool(const int index)> &func);
    void forAllCpuEfficientCores(const std::function<bool(const int index)> &func);


    void changeState(RefreshState newState);
    void startDataFetch();

    UIUpdatePackage prepareData(const legion::messages::HardwareMonitor latestHWMonitoringData,
                                const legion::messages::NvidiaNvml      latestNvidiaNvmlData,
                                const legion::messages::CPUInfo         cpuInfoData,
                                const legion::messages::HardwareMonitor hwMonitoringData,
                                const legion::messages::NvidiaNvml      nvidiaNvmlData,
                                const std::chrono::steady_clock::time_point   latestRefreshTime,
                                const bool isWindowFreqInfoByCoreVisible
                                );
signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);
private:

    Ui::HWMonitoring *ui;

    /*
     * Previos Data
     */
    legion::messages::HardwareMonitor                         m_hwMonitoringData;
    legion::messages::NvidiaNvml                              m_nvidiaNvmlData;

    /*
     * Previous Data for CPU Topology and Info
     */
    legion::messages::CPUTopology                             m_cpuTopology;
    legion::messages::CPUInfo                                 m_cpuInfoData;

    /*
     * Latest data for comparison
     */
    legion::messages::HardwareMonitor                         m_latestHWMonitoringData;
    legion::messages::NvidiaNvml                              m_latestNvidiaNvmlData;


    DataProvider                *m_dataProvider;
    CPUFrequency                *m_windowFreqInfoByCore;
    GPUDetails                  *m_windowGPUDetails;

    std::chrono::steady_clock::time_point    m_lastRefreshTime;

    int m_timerId            = -1;

    RefreshState m_currentState = RefreshState::IDLE;
    int m_pendingRequests       = 0;

    QFuture<legion::messages::HardwareMonitor> m_refreshFutureHWMon;
    QFuture<legion::messages::NvidiaNvml>      m_refreshFutureNvml;

    QFuture<UIUpdatePackage>                   m_refreshFutureUIUpdatePackage;

};

}
