// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "GPUDetails.h"
#include "ui_GPUDetails.h"


#include "HWMonitor.h"



namespace LenovoLegionGui {

GPUDetails::GPUDetails(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::GPUDetails)
{
    ui->setupUi(this);
}

void GPUDetails::init(const legion::messages::NvidiaNvml &nvidiaData)
{
    ui->groupBox_GPUDetails->setTitle(QString("GPU Details - %1").arg(nvidiaData.name()));

    ui->widget_GPUDetailsGPUUtilization->init("GPU [%1 %]",0,nvidiaData.hardware_monitor().gpu_utilization().max_value(),0,nvidiaData.hardware_monitor().power().max_value());
    ui->widget_GPUDetailsMemoryUtilization->init("Memory [%1 %]",0,nvidiaData.hardware_monitor().memory_utilization().max_value(),0,nvidiaData.hardware_monitor().memory_utilization().max_value());
    ui->widget_GPUDetailsMemoryUsed->init("Memory Used [%1 MB]",0,nvidiaData.hardware_monitor().memory_use().total() / (1024.0*1024.0),0,nvidiaData.hardware_monitor().memory_use().total() / (1024.0*1024.0*1024.0));

    ui->widget_GPUDetailsMemoryClock->init("Memory Clock [%1 MHz]",0,nvidiaData.hardware_monitor().memory_clock().max_value(),0,nvidiaData.hardware_monitor().memory_clock().max_value());
    ui->widget_GPUDetailsGraphicsClock->init("Core Clock [%1 MHz]",0,nvidiaData.hardware_monitor().gpu_clock().max_value(),0,nvidiaData.hardware_monitor().gpu_clock().max_value());
    ui->widget_GPUDetailsSMClock->init("SM Clock [%1 MHz]",0,nvidiaData.hardware_monitor().sm_clock().max_value(),0,nvidiaData.hardware_monitor().sm_clock().max_value());

    ui->widget_GPUDetailsPower->init("Power [%1 W]",0,nvidiaData.hardware_monitor().power().max_value() / 1000,0,nvidiaData.hardware_monitor().power().max_value() / 1000);
    ui->widget_GPUDetailsPCIERX->init("PCIe RX Throughput [%1 MB/s]",0,calculateTheoreticalMaxPcieThroughput(nvidiaData),0,calculateTheoreticalMaxPcieThroughput(nvidiaData));
    ui->widget_GPUDetailsPCIETX->init("PCIe TX Throughput [%1 MB/s]",0,calculateTheoreticalMaxPcieThroughput(nvidiaData),0,calculateTheoreticalMaxPcieThroughput(nvidiaData));
    ui->widget_GPUDetailsTemperature->init("Temperature [%1 °C]",0,nvidiaData.hardware_monitor().temperature().shutdown(),0,nvidiaData.hardware_monitor().temperature().shutdown());


    ui->label_GPUDetailsSummarBody->setText(
        "GPU Max Clock: " + QString::number(nvidiaData.hardware_monitor().gpu_clock().max_value()) + " MHz\n" +
        "GPU Min Clock: " + QString::number(nvidiaData.hardware_monitor().gpu_clock().min_value()) + " MHz\n" +
        "GPU Max SM Clock: " + QString::number(nvidiaData.hardware_monitor().sm_clock().max_value()) + " MHz\n" +
        "GPU Min SM Clock: " + QString::number(nvidiaData.hardware_monitor().sm_clock().min_value()) + " MHz\n" +
        "GPU Max Memory Clock: " + QString::number(nvidiaData.hardware_monitor().memory_clock().max_value()) + " MHz\n" +
        "GPU Min Memory Clock: " + QString::number(nvidiaData.hardware_monitor().memory_clock().min_value()) + " MHz\n" +
        "TDP Max: " + QString::number(nvidiaData.hardware_monitor().power().max_value() / 1000) + " W\n" +
        "TDP Min: " + QString::number(nvidiaData.hardware_monitor().power().min_value() / 1000) + " W\n" +
        "VRAM Size: " + QString::number(nvidiaData.hardware_monitor().memory_use().total() / (1024.0*1024.0*1024.0), 'f', 2) + " GB\n" +
        "PCIe Max Lanes: " + QString::number(nvidiaData.hardware_monitor().pcie().width_max()) + "\n" +
        "PCIe Max generation: " + QString::number(nvidiaData.hardware_monitor().pcie().generation_max()) + "\n" +
        "PCIe Theoretical Max Throughput: " + QString::number(calculateTheoreticalMaxPcieThroughput(nvidiaData)) + " MB/s\n" +
        "GPU Temperature Shutdown Threshold: " + QString::number(nvidiaData.hardware_monitor().temperature().shutdown()) + " °C\n"
        "GPU Temperature Slowdown Threshold: " + QString::number(nvidiaData.hardware_monitor().temperature().slowdown()) + " °C\n"
    );


}

void GPUDetails::refresh(const legion::messages::NvidiaNvml &nvidiaData)
{
    ui->widget_GPUDetailsGPUUtilization->refresh(nvidiaData.hardware_monitor().gpu_utilization().value() ,0,10,' ',QString("GPU Utilization: %1 %\n"\
                                                                                                           "GPU Utilization min: %2 %\n"\
                                                                                                           "GPU Utilization max: %3 %\n").arg(nvidiaData.hardware_monitor().gpu_utilization().value())
                                                                                                       .arg(nvidiaData.hardware_monitor().gpu_utilization().min_value())
                                                                                                       .arg(nvidiaData.hardware_monitor().gpu_utilization().max_value()));

    ui->widget_GPUDetailsMemoryUtilization->refresh(nvidiaData.hardware_monitor().memory_utilization().value() ,0,10,' ',QString("Memory Utilization: %1 %\n"\
                                                                                                           "Memory Utilization min: %2 %\n"\
                                                                                                           "Memory Utilization max: %3 %\n").arg(nvidiaData.hardware_monitor().memory_utilization().value())
                                                                                                       .arg(nvidiaData.hardware_monitor().memory_utilization().min_value())
                                                                                                       .arg(nvidiaData.hardware_monitor().memory_utilization().max_value()));
    ui->widget_GPUDetailsMemoryUsed->refresh(nvidiaData.hardware_monitor().memory_use().used() / (1024.0*1024.0) ,0,10,' ',QString("Memory Used: %1 MB\n"\
                                                                                                           "Memory Used min: %2 GB\n"\
                                                                                                           "Memory Used max: %3 GB\n").arg(nvidiaData.hardware_monitor().memory_use().used() / (1024.0*1024.0*1024.0))
                                                                                                       .arg(0)
                                                                                                       .arg(nvidiaData.hardware_monitor().memory_use().total() / (1024.0*1024.0*1024.0)));
    ui->widget_GPUDetailsMemoryClock->refresh(nvidiaData.hardware_monitor().memory_clock().value() ,0,10,' ',QString("Memory Clock: %1 MHz\n"\
                                                                                                           "Memory Clock min: %2 MHz\n"\
                                                                                                           "Memory Clock max: %3 MHz\n").arg(nvidiaData.hardware_monitor().memory_clock().value())
                                                                                                       .arg(nvidiaData.hardware_monitor().memory_clock().min_value())
                                                                                                       .arg(nvidiaData.hardware_monitor().memory_clock().max_value()));
    ui->widget_GPUDetailsGraphicsClock->refresh(nvidiaData.hardware_monitor().gpu_clock().value(),0,10,' ',QString("Core Clock: %1 MHz\n"\
                                                                                                           "Core Clock min: %2 MHz\n"\
                                                                                                           "Core Clock max: %3 MHz\n").arg(nvidiaData.hardware_monitor().gpu_clock().value())
                                                                                                       .arg(nvidiaData.hardware_monitor().gpu_clock().min_value())
                                                                                                       .arg(nvidiaData.hardware_monitor().gpu_clock().max_value()));
    ui->widget_GPUDetailsSMClock->refresh(nvidiaData.hardware_monitor().sm_clock().value(),0,10,' ',QString("SM Clock: %1 MHz\n"\
                                                                                                           "SM Clock min: %2 MHz\n"\
                                                                                                           "SM Clock max: %3 MHz\n").arg(nvidiaData.hardware_monitor().sm_clock().value())
                                                                                                       .arg(nvidiaData.hardware_monitor().sm_clock().min_value())
                                                                                                       .arg(nvidiaData.hardware_monitor().sm_clock().max_value()));


    ui->widget_GPUDetailsPower->refresh(nvidiaData.hardware_monitor().power().value() / 1000 ,0,10,' ',QString("Power: %1 W\n"\
                                                                                                           "Power min: %2 W\n"\
                                                                                                           "Power max: %3 W\n").arg(nvidiaData.hardware_monitor().power().value() / 1000)
                                                                                                       .arg(nvidiaData.hardware_monitor().power().min_value() / 1000)
                                                                                                       .arg(nvidiaData.hardware_monitor().power().max_value() / 1000));
    ui->widget_GPUDetailsPCIERX->refresh(nvidiaData.hardware_monitor().pcie().rx_bytes() / 1024 ,0,10,' ',QString("PCIe RX Throughput: %1 MB/s\n"\
                                                                                                           "PCIe RX Throughput min: %2 MB/s\n"\
                                                                                                           "PCIe RX Throughput max: %3 MB/s\n").arg(nvidiaData.hardware_monitor().pcie().rx_bytes()/1024.0)
                                                                                                       .arg(0)
                                                                                                       .arg(calculateTheoreticalMaxPcieThroughput(nvidiaData)));
    ui->widget_GPUDetailsPCIETX->refresh(nvidiaData.hardware_monitor().pcie().tx_bytes() / 1024 ,0,10,' ',QString("PCIe TX Throughput: %1 MB/s\n"\
                                                                                                                     "PCIe TX Throughput min: %2 MB/s\n"\
                                                                                                                     "PCIe TX Throughput max: %3 MB/s\n").arg(nvidiaData.hardware_monitor().pcie().tx_bytes()/1024.0)
                                                                                                                 .arg(0)
                                                                                                                 .arg(calculateTheoreticalMaxPcieThroughput(nvidiaData)));
    ui->widget_GPUDetailsTemperature->refresh(nvidiaData.hardware_monitor().temperature().value() ,0,10,' ',QString("Temperature: %1 °C\n"\
                                                                                                           "Temperature min: %2 °C\n"\
                                                                                                           "Temperature max: %3 °C\n").arg(nvidiaData.hardware_monitor().temperature().value())
                                                                                                       .arg(0)
                                                                                                       .arg(nvidiaData.hardware_monitor().temperature().shutdown()));


}

GPUDetails::~GPUDetails()
{
    delete ui;
}

void GPUDetails::closeEvent(QCloseEvent *event)
{
    emit closed();
    hide();
    event->ignore();
}

quint64 GPUDetails::calculateTheoreticalMaxPcieThroughput(const legion::messages::NvidiaNvml &nvidiaData)
{
    // PCIe Gen 3 x16 = 15.754 GB/s
    // PCIe Gen 4 x16 = 31.508 GB/s
    // PCIe Gen 5 x16 = 63.016 GB/s
    int pcieGen = nvidiaData.hardware_monitor().pcie().generation_max();;
    int pcieLanes = nvidiaData.hardware_monitor().pcie().width_max();

    double perLaneThroughputGBs = 0.0;
    switch (pcieGen) {
        case 1:
            perLaneThroughputGBs = 0.25; // 2.5 GT/s
            break;
        case 2:
            perLaneThroughputGBs = 0.5; // 5 GT/s
            break;
        case 3:
            perLaneThroughputGBs = 0.984; // 8 GT/s
            break;
        case 4:
            perLaneThroughputGBs = 1.969; // 16 GT/s
            break;
        case 5:
            perLaneThroughputGBs = 3.938; // 32 GT/s
            break;
        case 6:
            perLaneThroughputGBs = 7.877; // 64 GT/s
            break;
        default:
            perLaneThroughputGBs = 0.0;
            break;
    }

    double totalThroughputGBs = perLaneThroughputGBs * pcieLanes;
    return static_cast<quint64>(totalThroughputGBs * 1024); // Convert to MB/s
}

}
