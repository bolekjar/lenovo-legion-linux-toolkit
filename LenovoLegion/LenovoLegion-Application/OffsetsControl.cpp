#include "OffsetsControl.h"
#include "ui_OffsetsControl.h"

#include "MainWindow.h"

#include "DataProvider.h"

#include "../LenovoLegion-Daemon/SysFsDataProviderIntelMSR.h"
#include "../LenovoLegion-Daemon/DataProviderNvidiaNvml.h"

#include <QAbstractItemView>

namespace LenovoLegionGui {

const QString OffsetsControl::NAME = "Offset Control";;

const QMap<QString, legion::messages::CpuIntelMSR> OffsetsControl::CPU_VOLTAGE_PRESETS = {
    {"None",{}},
    {"Conservative",[](){
            legion::messages::CpuIntelMSR voltage;

            voltage.mutable_cpu()->set_offset(-50);
            voltage.mutable_uncore()->set_offset(-50);
            voltage.mutable_cache()->set_offset(-50);
            voltage.mutable_gpu()->set_offset(-40);
            voltage.mutable_analogio()->set_offset(0);

            return voltage;
        }(),},
    {"Moderate",[](){
         legion::messages::CpuIntelMSR voltage;

         voltage.mutable_cpu()->set_offset(-100);
         voltage.mutable_uncore()->set_offset(-80);
         voltage.mutable_cache()->set_offset(-100);
         voltage.mutable_gpu()->set_offset(-75);
         voltage.mutable_analogio()->set_offset(0);

         return voltage;
     }()},
    {"Aggressive",[](){
         legion::messages::CpuIntelMSR voltage;

         voltage.mutable_cpu()->set_offset(-150);
         voltage.mutable_uncore()->set_offset(-100);
         voltage.mutable_cache()->set_offset(-150);
         voltage.mutable_gpu()->set_offset(-100);
         voltage.mutable_analogio()->set_offset(0);

         return voltage;
     }()},
    {"Reset",[](){
         legion::messages::CpuIntelMSR voltage;

         voltage.mutable_cpu()->set_offset(0);
         voltage.mutable_uncore()->set_offset(0);
         voltage.mutable_cache()->set_offset(0);
         voltage.mutable_gpu()->set_offset(0);
         voltage.mutable_analogio()->set_offset(0);

         return voltage;
     }()},
};

const QMap<QString, legion::messages::NvidiaNvml> OffsetsControl::GPU_CLOCK_PRESETS{
    {"None",{}},
    {"Mild Overclock",[](){
            legion::messages::NvidiaNvml clock;

            clock.mutable_gpu_offset()->set_value(100);
            clock.mutable_memory_offset()->set_value(200);

            return clock;
        }(),},
    {"Moderate Overclock",[](){
         legion::messages::NvidiaNvml clock;

         clock.mutable_gpu_offset()->set_value(150);
         clock.mutable_memory_offset()->set_value(300);

         return clock;
     }()},
    {"Extreme Overclock",[](){
         legion::messages::NvidiaNvml clock;

         clock.mutable_gpu_offset()->set_value(200);
         clock.mutable_memory_offset()->set_value(500);

         return clock;
     }()},
    {"Reset",[](){
         legion::messages::NvidiaNvml clock;

         clock.mutable_gpu_offset()->set_value(0);
         clock.mutable_memory_offset()->set_value(0);
         return clock;
     }()}
};

OffsetsControl::OffsetsControl(DataProvider *dataProvider,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::OffsetsControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);


    ui->comboBox_GPUPreset->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_GPUPreset->view()->window()->setAttribute(Qt::WA_TranslucentBackground);

    ui->comboBox_cpuVoltagePreset->view()->window()->setWindowFlags( Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint | Qt::NoDropShadowWindowHint);
    ui->comboBox_cpuVoltagePreset->view()->window()->setAttribute(Qt::WA_TranslucentBackground);


    /*
     * Redad Data
     */
    readCpuIntelMsrData();
    readGpuNvmlData();

    /*
     * Validate Data
     */
    if(!m_cpuIntelMSRData.has_analogio() ||
       !m_cpuIntelMSRData.has_cache()    ||
       !m_cpuIntelMSRData.has_cpu()      ||
       !m_cpuIntelMSRData.has_gpu()      ||
       !m_cpuIntelMSRData.has_uncore()
       )
    {
        THROW_EXCEPTION(exception_T, ERROR_CODES::DATA_NOT_READY, "Voltage Control data not available");
    }


    /*
     * Setup Sliders
     */
    ui->horizontalSlider_cpuVoltageCore->blockSignals(true);
    ui->horizontalSlider_cpuVoltageCache->blockSignals(true);
    ui->horizontalSlider_cpuVoltageGPU->blockSignals(true);
    ui->horizontalSlider_cpuVoltageUncore->blockSignals(true);
    ui->horizontalSlider_cpuVoltageAnalogIO->blockSignals(true);

    ui->horizontalSlider_GPUClockOffset->blockSignals(true);
    ui->horizontalSlider_GPUMemoryOffset->blockSignals(true);

    ui->comboBox_cpuVoltagePreset->blockSignals(true);


    ui->horizontalSlider_cpuVoltageCore->setVisible(false);
    ui->horizontalSlider_cpuVoltageCache->setVisible(false);
    ui->horizontalSlider_cpuVoltageGPU->setVisible(false);
    ui->horizontalSlider_cpuVoltageUncore->setVisible(false);
    ui->horizontalSlider_cpuVoltageAnalogIO->setVisible(false);

    ui->horizontalSlider_GPUClockOffset->setVisible(false);
    ui->horizontalSlider_GPUMemoryOffset->setVisible(false);

    ui->comboBox_cpuVoltagePreset->setVisible(false);
    ui->comboBox_GPUPreset->setVisible(false);
    ui->groupBox_CPUVoltageOffsets->setVisible(false);
    ui->groupBox_GPUOffsets->setVisible(false);

    ui->label_cpuVoltageCore->setVisible(false);
    ui->label_cpuVoltageCache->setVisible(false);
    ui->label_cpuVoltageGPU->setVisible(false);
    ui->label_cpuVoltageUncore->setVisible(false);
    ui->label_cpuVoltageAnalogIO->setVisible(false);

    ui->label_GPUClockOffset->setVisible(false);
    ui->label_GPUMemoryOffset->setVisible(false);

    ui->lcdNumber_cpuVoltageCore->setVisible(false);
    ui->lcdNumber_cpuVoltageCache->setVisible(false);
    ui->lcdNumber_cpuVoltageGPU->setVisible(false);
    ui->lcdNumber_cpuVoltageUncore->setVisible(false);
    ui->lcdNumber_cpuVoltageAnalogIO->setVisible(false);

    ui->lcdNumber_GPUClockOffset->setVisible(false);
    ui->lcdNumber_GPUMemoryOffset->setVisible(false);

    ui->pushButton_cpuVoltageApply->setVisible(false);
    ui->pushButton_cpuVoltageCancel->setVisible(false);


    ui->pushButton_GPUApply->setVisible(false);
    ui->pushButton_GPUCancel->setVisible(false);





    for (auto presetNameI = CPU_VOLTAGE_PRESETS.constBegin(); presetNameI != CPU_VOLTAGE_PRESETS.constEnd(); ++presetNameI)
    {
        ui->comboBox_cpuVoltagePreset->addItem(presetNameI.key());
    };


    for (auto presetNameI = GPU_CLOCK_PRESETS.constBegin(); presetNameI != GPU_CLOCK_PRESETS.constEnd(); ++presetNameI)
    {
        ui->comboBox_GPUPreset->addItem(presetNameI.key());
    };

    if(m_cpuIntelMSRData.cpu().supported())
    {
        ui->horizontalSlider_cpuVoltageCore->setMinimum(-m_cpuIntelMSRData.cpu().max_undervolt());
        ui->horizontalSlider_cpuVoltageCore->setMaximum(m_cpuIntelMSRData.cpu().max_overvolt());

        ui->horizontalSlider_cpuVoltageCore->setVisible(true);
        ui->comboBox_cpuVoltagePreset->setVisible(true);
        ui->groupBox_CPUVoltageOffsets->setVisible(true);
        ui->lcdNumber_cpuVoltageCore->setVisible(true);
        ui->label_cpuVoltageCore->setVisible(true);


        ui->pushButton_cpuVoltageApply->setVisible(true);
        ui->pushButton_cpuVoltageCancel->setVisible(true);

        ui->horizontalSlider_cpuVoltageCore->blockSignals(false);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
    }

    if(m_cpuIntelMSRData.cache().supported())
    {
        ui->horizontalSlider_cpuVoltageCache->setMinimum(-m_cpuIntelMSRData.cache().max_undervolt());
        ui->horizontalSlider_cpuVoltageCache->setMaximum(m_cpuIntelMSRData.cache().max_overvolt());

        ui->horizontalSlider_cpuVoltageCache->setVisible(true);
        ui->comboBox_cpuVoltagePreset->setVisible(true);
        ui->groupBox_CPUVoltageOffsets->setVisible(true);
        ui->lcdNumber_cpuVoltageCache->setVisible(true);
        ui->label_cpuVoltageCache->setVisible(true);


        ui->pushButton_cpuVoltageApply->setVisible(true);
        ui->pushButton_cpuVoltageCancel->setVisible(true);

        ui->horizontalSlider_cpuVoltageCache->blockSignals(false);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
    }


    if(m_cpuIntelMSRData.gpu().supported())
    {
        ui->horizontalSlider_cpuVoltageGPU->setMinimum(-m_cpuIntelMSRData.gpu().max_undervolt());
        ui->horizontalSlider_cpuVoltageGPU->setMaximum(m_cpuIntelMSRData.gpu().max_overvolt());

        ui->horizontalSlider_cpuVoltageGPU->setVisible(true);
        ui->comboBox_cpuVoltagePreset->setVisible(true);
        ui->groupBox_CPUVoltageOffsets->setVisible(true);
        ui->lcdNumber_cpuVoltageGPU->setVisible(true);
        ui->label_cpuVoltageGPU->setVisible(true);


        ui->pushButton_cpuVoltageApply->setVisible(true);
        ui->pushButton_cpuVoltageCancel->setVisible(true);

        ui->horizontalSlider_cpuVoltageGPU->blockSignals(false);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
    }


    if(m_cpuIntelMSRData.uncore().supported())
    {
        ui->horizontalSlider_cpuVoltageUncore->setMinimum(-m_cpuIntelMSRData.uncore().max_undervolt());
        ui->horizontalSlider_cpuVoltageUncore->setMaximum(m_cpuIntelMSRData.uncore().max_overvolt());


        ui->horizontalSlider_cpuVoltageUncore->setVisible(true);
        ui->comboBox_cpuVoltagePreset->setVisible(true);
        ui->groupBox_CPUVoltageOffsets->setVisible(true);
        ui->lcdNumber_cpuVoltageUncore->setVisible(true);
        ui->label_cpuVoltageUncore->setVisible(true);

        ui->horizontalSlider_cpuVoltageUncore->blockSignals(false);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
    }

    if(m_cpuIntelMSRData.analogio().supported())
    {
        ui->horizontalSlider_cpuVoltageAnalogIO->setMinimum(-m_cpuIntelMSRData.analogio().max_undervolt());
        ui->horizontalSlider_cpuVoltageAnalogIO->setMaximum(m_cpuIntelMSRData.analogio().max_overvolt());

        ui->horizontalSlider_cpuVoltageAnalogIO->setVisible(true);
        ui->comboBox_cpuVoltagePreset->setVisible(true);
        ui->groupBox_CPUVoltageOffsets->setVisible(true);
        ui->lcdNumber_cpuVoltageAnalogIO->setVisible(true);
        ui->label_cpuVoltageAnalogIO->setVisible(true);


        ui->pushButton_cpuVoltageApply->setVisible(true);
        ui->pushButton_cpuVoltageCancel->setVisible(true);


        ui->horizontalSlider_cpuVoltageAnalogIO->blockSignals(false);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
    }

    if(m_gpuNvmlData.has_gpu_offset())
    {
        ui->horizontalSlider_GPUClockOffset->setMinimum(m_gpuNvmlData.gpu_offset().min());
        ui->horizontalSlider_GPUClockOffset->setMaximum(m_gpuNvmlData.gpu_offset().max());

        ui->horizontalSlider_GPUClockOffset->setVisible(true);
        ui->groupBox_GPUOffsets->setVisible(true);
        ui->comboBox_GPUPreset->setVisible(true);
        ui->lcdNumber_GPUClockOffset->setVisible(true);
        ui->label_GPUClockOffset->setVisible(true);


        ui->pushButton_GPUApply->setVisible(true);
        ui->pushButton_GPUCancel->setVisible(true);

        ui->horizontalSlider_GPUClockOffset->blockSignals(false);
        ui->comboBox_GPUPreset->blockSignals(false);
    }

    if(m_gpuNvmlData.has_memory_offset())
    {
        ui->horizontalSlider_GPUMemoryOffset->setMinimum(m_gpuNvmlData.memory_offset().min());
        ui->horizontalSlider_GPUMemoryOffset->setMaximum(m_gpuNvmlData.memory_offset().max());

        ui->horizontalSlider_GPUMemoryOffset->setVisible(true);
        ui->groupBox_GPUOffsets->setVisible(true);
        ui->comboBox_GPUPreset->setVisible(true);
        ui->lcdNumber_GPUMemoryOffset->setVisible(true);
        ui->label_GPUMemoryOffset->setVisible(true);

        ui->pushButton_GPUApply->setVisible(true);
        ui->pushButton_GPUCancel->setVisible(true);

        ui->horizontalSlider_GPUMemoryOffset->blockSignals(false);
        ui->comboBox_GPUPreset->blockSignals(false);
    }

    refresh();
}

OffsetsControl::~OffsetsControl()
{
    delete ui;
}

void OffsetsControl::refresh()
{
    refreshCpuVoltageOffsetData();
    refreshGpuOffsetData();

    markChanges();
}

void OffsetsControl::on_horizontalSlider_cpuVoltageCore_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_cpuVoltageCore->display(value);
        markChangesCpuVoltageOffsetData();
        return;
    }

    ui->horizontalSlider_cpuVoltageCore->setValue(value - (value % 10));

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::on_horizontalSlider_cpuVoltageUncore_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_cpuVoltageUncore->display(value);
        markChangesCpuVoltageOffsetData();
        return;
    }

    ui->horizontalSlider_cpuVoltageUncore->setValue(value - (value % 10));
}

void OffsetsControl::on_horizontalSlider_cpuVoltageCache_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_cpuVoltageCache->display(value);
        markChangesCpuVoltageOffsetData();
        return;
    }

    ui->horizontalSlider_cpuVoltageCache->setValue(value - (value % 10));

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::on_horizontalSlider_cpuVoltageGPU_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_cpuVoltageGPU->display(value);
        markChangesCpuVoltageOffsetData();
        return;
    }

    ui->horizontalSlider_cpuVoltageGPU->setValue(value - (value % 10));

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::on_horizontalSlider_cpuVoltageAnalogIO_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_cpuVoltageAnalogIO->display(value);
        markChangesCpuVoltageOffsetData();
        return;
    }

    ui->horizontalSlider_cpuVoltageAnalogIO->setValue(value - (value % 10));

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::on_pushButton_cpuVoltageCancel_clicked()
{
    refreshCpuVoltageOffsetData();
}

void OffsetsControl::on_pushButton_cpuVoltageApply_clicked()
{
    legion::messages::CpuIntelMSR cpuIntelMSR;

    if(!ui->horizontalSlider_cpuVoltageCore->isHidden())
        cpuIntelMSR.mutable_cpu()->set_offset(ui->horizontalSlider_cpuVoltageCore->value() * 1000);
    if(!ui->horizontalSlider_cpuVoltageUncore->isHidden())
        cpuIntelMSR.mutable_uncore()->set_offset(ui->horizontalSlider_cpuVoltageUncore->value() * 1000);
    if(!ui->horizontalSlider_cpuVoltageCache->isHidden())
        cpuIntelMSR.mutable_cache()->set_offset(ui->horizontalSlider_cpuVoltageCache->value() * 1000);
    if(!ui->horizontalSlider_cpuVoltageGPU->isHidden())
        cpuIntelMSR.mutable_gpu()->set_offset(ui->horizontalSlider_cpuVoltageGPU->value()* 1000);
    if(!ui->horizontalSlider_cpuVoltageAnalogIO->isHidden())
        cpuIntelMSR.mutable_analogio()->set_offset(ui->horizontalSlider_cpuVoltageAnalogIO->value() * 1000);

    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderIntelMSR::dataType,cpuIntelMSR);

    readCpuIntelMsrData();

    refreshCpuVoltageOffsetData();
}

void OffsetsControl::markChangesCpuVoltageOffsetData()
{
    if(static_cast<int>(ui->horizontalSlider_cpuVoltageCore->value()) != m_cpuIntelMSRData.cpu().offset())
    {
        ui->label_cpuVoltageCore->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_cpuVoltageCore->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<int>(ui->horizontalSlider_cpuVoltageUncore->value()) != m_cpuIntelMSRData.uncore().offset())
    {
        ui->label_cpuVoltageUncore->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_cpuVoltageUncore->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<int>(ui->horizontalSlider_cpuVoltageCache->value()) != m_cpuIntelMSRData.cache().offset())
    {
        ui->label_cpuVoltageCache->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_cpuVoltageCache->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<int>(ui->horizontalSlider_cpuVoltageGPU->value()) != m_cpuIntelMSRData.gpu().offset())
    {
        ui->label_cpuVoltageGPU->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_cpuVoltageGPU->setStyleSheet(QString("QLabel { }"));
    }

    if(static_cast<int>(ui->horizontalSlider_cpuVoltageAnalogIO->value()) != m_cpuIntelMSRData.analogio().offset())
    {
        ui->label_cpuVoltageAnalogIO->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_cpuVoltageAnalogIO->setStyleSheet(QString("QLabel { }"));
    }
}

void OffsetsControl::markChangesGpuOffsetData()
{
    if(ui->horizontalSlider_GPUClockOffset->value() != m_gpuNvmlData.gpu_offset().value())
    {
        ui->label_GPUClockOffset->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_GPUClockOffset->setStyleSheet(QString("QLabel { }"));
    }

    if(ui->horizontalSlider_GPUMemoryOffset->value() != m_gpuNvmlData.memory_offset().value())
    {
        ui->label_GPUMemoryOffset->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_GPUMemoryOffset->setStyleSheet(QString("QLabel { }"));
    }
}

void OffsetsControl::markChanges()
{
    markChangesCpuVoltageOffsetData();
    markChangesGpuOffsetData();
}

void OffsetsControl::readCpuIntelMsrData()
{
    m_cpuIntelMSRData = m_dataProvider->getDataMessage<legion::messages::CpuIntelMSR> (LenovoLegionDaemon::SysFsDataProviderIntelMSR::dataType);

    /*
     * Normalize => mV
     */
    m_cpuIntelMSRData.mutable_cpu()->set_offset((m_cpuIntelMSRData.cpu().offset() > 0 ? m_cpuIntelMSRData.cpu().offset() + 999 :m_cpuIntelMSRData.cpu().offset() - 999)/ 1000);
    m_cpuIntelMSRData.mutable_cpu()->set_max_undervolt((m_cpuIntelMSRData.cpu().max_undervolt() + 999 )/ 1000);
    m_cpuIntelMSRData.mutable_cpu()->set_max_overvolt((m_cpuIntelMSRData.cpu().max_overvolt() + 999 )/ 1000);

    m_cpuIntelMSRData.mutable_uncore()->set_offset((m_cpuIntelMSRData.uncore().offset() > 0 ? m_cpuIntelMSRData.uncore().offset() + 999 : m_cpuIntelMSRData.uncore().offset() - 999) / 1000);
    m_cpuIntelMSRData.mutable_uncore()->set_max_undervolt((m_cpuIntelMSRData.uncore().max_undervolt() + 999) / 1000);
    m_cpuIntelMSRData.mutable_uncore()->set_max_overvolt((m_cpuIntelMSRData.uncore().max_overvolt() + 999) / 1000);

    m_cpuIntelMSRData.mutable_cache()->set_offset((m_cpuIntelMSRData.cache().offset() > 0 ? m_cpuIntelMSRData.cache().offset() + 999 : m_cpuIntelMSRData.cache().offset() - 999) / 1000);
    m_cpuIntelMSRData.mutable_cache()->set_max_undervolt((m_cpuIntelMSRData.cache().max_undervolt() + 999) / 1000);
    m_cpuIntelMSRData.mutable_cache()->set_max_overvolt((m_cpuIntelMSRData.cache().max_overvolt() + 999) / 1000);

    m_cpuIntelMSRData.mutable_gpu()->set_offset((m_cpuIntelMSRData.gpu().offset() > 0 ? m_cpuIntelMSRData.gpu().offset()  + 999 : m_cpuIntelMSRData.gpu().offset() - 999) / 1000);
    m_cpuIntelMSRData.mutable_gpu()->set_max_undervolt((m_cpuIntelMSRData.gpu().max_undervolt() + 999) / 1000);
    m_cpuIntelMSRData.mutable_gpu()->set_max_overvolt((m_cpuIntelMSRData.gpu().max_overvolt() + 999) / 1000);

    m_cpuIntelMSRData.mutable_analogio()->set_offset((m_cpuIntelMSRData.analogio().offset() > 0 ?m_cpuIntelMSRData.analogio().offset() + 999 : m_cpuIntelMSRData.analogio().offset() - 999)/ 1000);
    m_cpuIntelMSRData.mutable_analogio()->set_max_undervolt((m_cpuIntelMSRData.analogio().max_undervolt() + 999) / 1000);
    m_cpuIntelMSRData.mutable_analogio()->set_max_overvolt((m_cpuIntelMSRData.analogio().max_overvolt() + 999) / 1000);
}

void OffsetsControl::readGpuNvmlData()
{
    m_gpuNvmlData = m_dataProvider->getDataMessage<legion::messages::NvidiaNvml>(LenovoLegionDaemon::DataProviderNvidiaNvml::dataType);
}

void OffsetsControl::on_comboBox_cpuVoltagePreset_currentTextChanged(const QString &arg1)
{
    const legion::messages::CpuIntelMSR &preset = CPU_VOLTAGE_PRESETS.value(arg1);

    if(preset.has_cpu() && m_cpuIntelMSRData.cpu().supported())
        ui->horizontalSlider_cpuVoltageCore->setValue(preset.cpu().offset());
    if(preset.has_uncore() && m_cpuIntelMSRData.uncore().supported())
        ui->horizontalSlider_cpuVoltageUncore->setValue(preset.uncore().offset());
    if(preset.has_cache() && m_cpuIntelMSRData.cache().supported())
        ui->horizontalSlider_cpuVoltageCache->setValue(preset.cache().offset());
    if(preset.has_gpu() && m_cpuIntelMSRData.gpu().supported())
        ui->horizontalSlider_cpuVoltageGPU->setValue(preset.gpu().offset());
    if(preset.has_analogio() && m_cpuIntelMSRData.analogio().supported())
        ui->horizontalSlider_cpuVoltageAnalogIO->setValue(preset.analogio().offset());

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::refreshCpuVoltageOffsetData()
{
    ui->horizontalSlider_cpuVoltageCore->blockSignals(true);
    ui->horizontalSlider_cpuVoltageCache->blockSignals(true);
    ui->horizontalSlider_cpuVoltageGPU->blockSignals(true);
    ui->horizontalSlider_cpuVoltageUncore->blockSignals(true);
    ui->horizontalSlider_cpuVoltageAnalogIO->blockSignals(true);

    ui->comboBox_cpuVoltagePreset->blockSignals(true);

    ui->horizontalSlider_cpuVoltageCore->setEnabled(false);
    ui->horizontalSlider_cpuVoltageCache->setEnabled(false);
    ui->horizontalSlider_cpuVoltageGPU->setEnabled(false);
    ui->horizontalSlider_cpuVoltageUncore->setEnabled(false);
    ui->horizontalSlider_cpuVoltageAnalogIO->setEnabled(false);

    ui->comboBox_cpuVoltagePreset->setEnabled(false);


    ui->lcdNumber_cpuVoltageCore->blockSignals(true);
    ui->lcdNumber_cpuVoltageCache->blockSignals(true);
    ui->lcdNumber_cpuVoltageGPU->blockSignals(true);
    ui->lcdNumber_cpuVoltageUncore->blockSignals(true);
    ui->lcdNumber_cpuVoltageAnalogIO->blockSignals(true);

    ui->lcdNumber_cpuVoltageCore->setEnabled(false);
    ui->lcdNumber_cpuVoltageCache->setEnabled(false);
    ui->lcdNumber_cpuVoltageGPU->setEnabled(false);
    ui->lcdNumber_cpuVoltageUncore->setEnabled(false);
    ui->lcdNumber_cpuVoltageAnalogIO->setEnabled(false);

    ui->horizontalSlider_cpuVoltageCore->setValue(0);
    ui->horizontalSlider_cpuVoltageCache->setValue(0);
    ui->horizontalSlider_cpuVoltageGPU->setValue(0);
    ui->horizontalSlider_cpuVoltageUncore->setValue(0);
    ui->horizontalSlider_cpuVoltageAnalogIO->setValue(0);

    ui->lcdNumber_cpuVoltageCore->display(0);
    ui->lcdNumber_cpuVoltageCache->display(0);
    ui->lcdNumber_cpuVoltageGPU->display(0);
    ui->lcdNumber_cpuVoltageUncore->display(0);
    ui->lcdNumber_cpuVoltageAnalogIO->display(0);

    ui->comboBox_cpuVoltagePreset->setCurrentText("None");


    if(!ui->horizontalSlider_cpuVoltageCore->isHidden())
    {
        ui->horizontalSlider_cpuVoltageCore->setValue(m_cpuIntelMSRData.cpu().offset());
        ui->lcdNumber_cpuVoltageCore->display(m_cpuIntelMSRData.cpu().offset());

        ui->horizontalSlider_cpuVoltageCore->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
        ui->horizontalSlider_cpuVoltageCore->blockSignals(false);
        ui->lcdNumber_cpuVoltageCore->blockSignals(false);
        ui->lcdNumber_cpuVoltageCore->setEnabled(true);
    }

    if(!ui->horizontalSlider_cpuVoltageCache->isHidden())
    {
        ui->horizontalSlider_cpuVoltageCache->setValue(m_cpuIntelMSRData.cache().offset());
        ui->lcdNumber_cpuVoltageCache->display(m_cpuIntelMSRData.cache().offset());

        ui->horizontalSlider_cpuVoltageCache->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
        ui->horizontalSlider_cpuVoltageCache->blockSignals(false);
        ui->lcdNumber_cpuVoltageCache->blockSignals(false);
        ui->lcdNumber_cpuVoltageCache->setEnabled(true);
    }

    if(!ui->horizontalSlider_cpuVoltageGPU->isHidden())
    {
        ui->horizontalSlider_cpuVoltageGPU->setValue(m_cpuIntelMSRData.gpu().offset());
        ui->lcdNumber_cpuVoltageGPU->display(m_cpuIntelMSRData.gpu().offset());

        ui->horizontalSlider_cpuVoltageGPU->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
        ui->horizontalSlider_cpuVoltageGPU->blockSignals(false);
        ui->lcdNumber_cpuVoltageGPU->blockSignals(false);
        ui->lcdNumber_cpuVoltageGPU->setEnabled(true);
    }

    if(!ui->horizontalSlider_cpuVoltageUncore->isHidden())
    {
        ui->horizontalSlider_cpuVoltageUncore->setValue(m_cpuIntelMSRData.uncore().offset());
        ui->lcdNumber_cpuVoltageUncore->display(m_cpuIntelMSRData.uncore().offset());

        ui->horizontalSlider_cpuVoltageUncore->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
        ui->horizontalSlider_cpuVoltageUncore->blockSignals(false);
        ui->lcdNumber_cpuVoltageUncore->blockSignals(false);
        ui->lcdNumber_cpuVoltageUncore->setEnabled(true);
    }

    if(!ui->horizontalSlider_cpuVoltageAnalogIO->isHidden())
    {
        ui->horizontalSlider_cpuVoltageAnalogIO->setValue(m_cpuIntelMSRData.analogio().offset());
        ui->lcdNumber_cpuVoltageAnalogIO->display(m_cpuIntelMSRData.analogio().offset());

        ui->horizontalSlider_cpuVoltageAnalogIO->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->setEnabled(true);
        ui->comboBox_cpuVoltagePreset->blockSignals(false);
        ui->horizontalSlider_cpuVoltageAnalogIO->blockSignals(false);
        ui->lcdNumber_cpuVoltageAnalogIO->blockSignals(false);
        ui->lcdNumber_cpuVoltageAnalogIO->setEnabled(true);
    }

    markChangesCpuVoltageOffsetData();
}

void OffsetsControl::refreshGpuOffsetData()
{
    ui->comboBox_GPUPreset->setCurrentText("None");

    if(!ui->horizontalSlider_GPUClockOffset->isHidden())
    {
        ui->horizontalSlider_GPUClockOffset->setValue(m_gpuNvmlData.gpu_offset().value());
        ui->lcdNumber_GPUClockOffset->display(m_gpuNvmlData.gpu_offset().value());
    }

    if(!ui->horizontalSlider_GPUMemoryOffset->isHidden())
    {
        ui->horizontalSlider_GPUMemoryOffset->setValue(m_gpuNvmlData.memory_offset().value());
        ui->lcdNumber_GPUMemoryOffset->display(m_gpuNvmlData.memory_offset().value());
    }

    markChangesGpuOffsetData();
}

void OffsetsControl::on_pushButton_GPUCancel_clicked()
{
    refreshGpuOffsetData();
}

void OffsetsControl::on_pushButton_GPUApply_clicked()
{
    legion::messages::NvidiaNvml gpuNvml;

    if(!ui->horizontalSlider_GPUClockOffset->isHidden())
    {
        gpuNvml.mutable_gpu_offset()->set_value(ui->horizontalSlider_GPUClockOffset->value());
    }

    if(!ui->horizontalSlider_GPUMemoryOffset->isHidden())
    {
       gpuNvml.mutable_memory_offset()->set_value(ui->horizontalSlider_GPUMemoryOffset->value());
    }

    m_dataProvider->setDataMessage(LenovoLegionDaemon::DataProviderNvidiaNvml::dataType,gpuNvml);

    readGpuNvmlData();

    refreshGpuOffsetData();
}

void OffsetsControl::on_comboBox_GPUPreset_currentTextChanged(const QString &arg1)
{
    const legion::messages::NvidiaNvml &preset = GPU_CLOCK_PRESETS.value(arg1);

    if(preset.has_gpu_offset())
        ui->horizontalSlider_GPUClockOffset->setValue(preset.gpu_offset().value());
    if(preset.has_memory_offset())
        ui->horizontalSlider_GPUMemoryOffset->setValue(preset.memory_offset().value());

    markChangesGpuOffsetData();
}

void OffsetsControl::on_horizontalSlider_GPUMemoryOffset_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_GPUMemoryOffset->display(value);
        markChangesGpuOffsetData();
        return;
    }

    ui->horizontalSlider_GPUMemoryOffset->setValue(value - (value % 10));

    markChangesGpuOffsetData();
}

void OffsetsControl::on_horizontalSlider_GPUClockOffset_valueChanged(int value)
{
    if((value % 10) == 0)
    {
        ui->lcdNumber_GPUClockOffset->display(value);
        markChangesGpuOffsetData();
        return;
    }

    ui->horizontalSlider_GPUClockOffset->setValue(value - (value % 10));

    markChangesGpuOffsetData();
}







}
