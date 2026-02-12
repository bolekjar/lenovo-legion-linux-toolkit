// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"
#include "../LenovoLegion-PrepareBuild/CPUTopology.pb.h"


#include <QWidget>
#include <QGroupBox>


namespace Ui {
class CPUFrequency;
}

class QGridLayout;

namespace LenovoLegionGui {

    class CPUFrequency : public QWidget
    {
        Q_OBJECT

    public:
        explicit CPUFrequency(QWidget *parent = nullptr);
        ~CPUFrequency();


        void init(const legion::messages::CPUInfo& data,const legion::messages::CPUTopology& topology,int freqMin,int freqMax);

        void addPerformanceWidget(int i, QWidget *widget);
        void addEfficiencyWidget(int i,  QWidget *widget);

        QWidget* getPerforamnceWidget(int i);
        QWidget* getEfficiencyWidget(int i);


    signals:

        void closed();

    protected:

        void closeEvent(QCloseEvent *event) override;


    private:
        Ui::CPUFrequency *ui;
        QGridLayout      *m_gridLayoutCPUEfficient;
    };
}
