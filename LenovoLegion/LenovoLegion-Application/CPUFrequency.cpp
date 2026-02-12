// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "CPUFrequency.h"
#include "ui_CPUFrequency.h"

#include <QCloseEvent>

namespace LenovoLegionGui {

    CPUFrequency::CPUFrequency(QWidget *parent)
        : QWidget(parent)
        , ui(new Ui::CPUFrequency)
        , m_gridLayoutCPUEfficient(nullptr)
    {
        ui->setupUi(this);
    }

    CPUFrequency::~CPUFrequency()
    {
        delete ui;
    }

    void CPUFrequency::init(const legion::messages::CPUInfo &data,const legion::messages::CPUTopology& topology,int freqMin,int freqMax)
    {
        ui->groupBox_CPUDetails->setTitle(QString("CPU Details (%1)").arg(QString(data.cpu_model().data()).trimmed()));

        ui->label_CPUDetails->setText(QString(
                                          "CPU Generation: %1\n"\
                                          "CPU Architecture: %2\n"\
                                          "CPU Cores: %3\n"\
                                          "CPU Min Frequency: %4 MHz\n"\
                                          "CPU Max Frequency: %5 MHz\n"\
                                          "CPU Cache L%6,%7: %8 \n"\
                                          "CPU Cache L%9,%10: %11 \n"\
                                          "CPU Cache L%12,%13: %14 \n"\
                                          "CPU Cache L%15,%16: %17 \n"\
                                          "CPU Physical Package Id: %18").arg(QString(data.cpu_generation().data()).trimmed())
                                          .arg(QString(data.architecture().data()).trimmed())
                                          .arg(topology.active_cpus().size() > 0 ? topology.active_cpus().at(0).cpu_max() + 1  : -1)
                                          .arg(freqMin)
                                          .arg(freqMax)
                                          .arg(data.caches().size() > 0 ? data.caches().at(0).level() : -1)
                                          .arg(data.caches().size() > 0 ? data.caches().at(0).type() : "")
                                          .arg(data.caches().size() > 0 ? data.caches().at(0).size() : "-1")

                                          .arg(data.caches().size() > 1 ? data.caches().at(1).level() : -1)
                                          .arg(data.caches().size() > 1 ? data.caches().at(1).type() : "")
                                          .arg(data.caches().size() > 1 ? data.caches().at(1).size() : "-1")

                                          .arg(data.caches().size() > 2 ? data.caches().at(2).level() : -1)
                                          .arg(data.caches().size() > 2 ? data.caches().at(2).type() : "")
                                          .arg(data.caches().size() > 2 ? data.caches().at(2).size() : "-1")

                                          .arg(data.caches().size() > 3 ? data.caches().at(3).level() : -1)
                                          .arg(data.caches().size() > 3 ? data.caches().at(3).type() : "")
                                          .arg(data.caches().size() > 3 ? data.caches().at(3).size() : "-1")
                                          .arg(data.physical_package_id())
                                      );

    }

    void CPUFrequency::addPerformanceWidget(int i, QWidget *widget)
    {
        ui->gridLayout->addWidget(widget,i % 8,i/8);
    }

    void CPUFrequency::addEfficiencyWidget(int i, QWidget *widget)
    {
        ui->gridLayout->addWidget(widget,i % 8,i/8);
    }

    QWidget *CPUFrequency::getPerforamnceWidget(int i)
    {
        return ui->gridLayout->itemAtPosition(i % 8,i/8)->widget();
    }

    QWidget *CPUFrequency::getEfficiencyWidget(int i)
    {
        return ui->gridLayout->itemAtPosition(i % 8,i/8)->widget();
    }

    void CPUFrequency::closeEvent(QCloseEvent *event)
    {
        emit closed();
        hide();
        event->ignore();
    }
}
