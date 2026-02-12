// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QWidget>
#include "WidgetMessage.h"

#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-PrepareBuild/CpuIntelMSR.pb.h"
#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"


namespace Ui {
class OffsetsControl;
}


namespace LenovoLegionGui {

class DataProvider;
class OffsetsControl : public QWidget
{
    Q_OBJECT

public:
    DEFINE_EXCEPTION(VoltageControl)

    enum ERROR_CODES : int {
        DATA_NOT_READY = -1
    };


    static const QString              NAME;
    static const QMap<QString,legion::messages::CpuIntelMSR> CPU_VOLTAGE_PRESETS;
    static const QMap<QString,legion::messages::NvidiaNvml>  GPU_CLOCK_PRESETS;

public:
    explicit OffsetsControl(DataProvider *dataProvider ,QWidget *parent = nullptr);
    ~OffsetsControl();

    void refresh();

signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private slots:
    void on_horizontalSlider_cpuVoltageCore_valueChanged(int value);

    void on_horizontalSlider_cpuVoltageUncore_valueChanged(int value);

    void on_horizontalSlider_cpuVoltageCache_valueChanged(int value);

    void on_horizontalSlider_cpuVoltageGPU_valueChanged(int value);

    void on_horizontalSlider_cpuVoltageAnalogIO_valueChanged(int value);

    void on_pushButton_cpuVoltageCancel_clicked();

    void on_pushButton_cpuVoltageApply_clicked();

    void on_comboBox_cpuVoltagePreset_currentTextChanged(const QString &arg1);

    void on_pushButton_GPUCancel_clicked();

    void on_pushButton_GPUApply_clicked();

    void on_comboBox_GPUPreset_currentTextChanged(const QString &arg1);

    void on_horizontalSlider_GPUMemoryOffset_valueChanged(int value);

    void on_horizontalSlider_GPUClockOffset_valueChanged(int value);

private:

    void refreshCpuVoltageOffsetData();
    void refreshGpuOffsetData();

    void markChangesCpuVoltageOffsetData();
    void markChangesGpuOffsetData();
    void markChanges();

    void readCpuIntelMsrData();
    void readGpuNvmlData();

private:
    Ui::OffsetsControl *ui;

    legion::messages::CpuIntelMSR m_cpuIntelMSRData;
    legion::messages::NvidiaNvml  m_gpuNvmlData;

    DataProvider *m_dataProvider;
};

}
