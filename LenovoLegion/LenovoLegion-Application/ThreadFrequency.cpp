// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ThreadFrequency.h"
#include "ui_ThreadFrequency.h"

namespace LenovoLegionGui {

ThreadFrequency::ThreadFrequency(const QString &title, int min, int max, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ThreadFrequency)
    , m_chart(new QChart())
    , m_barSet(new QBarSet(""))
    , m_title(title)
{
    ui->setupUi(this);

    m_barSet->setColor(QColor(Qt::white));

    for(int i = 0; i < 100; ++i) {
        *m_barSet << min;
    }


    auto series = new QStackedBarSeries;

    series->append(m_barSet);
    series->setBarWidth(1.0);

    m_chart->addSeries(series);
    m_chart->createDefaultAxes();
    m_chart->legend()->setVisible(false);
    m_chart->setTitle(m_title);

    auto axisV = m_chart->axes(Qt::Vertical);
    auto axisH = m_chart->axes(Qt::Horizontal);


    axisV.first()->setRange(min, max);
    axisV.first()->setLabelsVisible(false);
    axisV.first()->setGridLineVisible(true);
    axisV.first()->setLineVisible(false);


    axisH.first()->setRange(0, 100);
    axisH.first()->setLabelsVisible(false);
    axisH.first()->setGridLineVisible(false);
    axisH.first()->setLineVisible(false);


    m_chart->setMargins(QMargins(5,5,5,5));
    m_chart->setContentsMargins(0,0,0,0);
    m_chart->setBackgroundBrush(QBrush(0x999999));

    auto charView = new QChartView(m_chart,this);

    charView->setContentsMargins(0,0,0,0);
    charView->setRenderHint(QPainter::Antialiasing);

    ui->horizontalLayout_ThreadFrequency->addWidget(charView);
    ui->horizontalLayout_ThreadFrequency->setContentsMargins(0,0,0,0);

    refreshToolTip();
}

ThreadFrequency::~ThreadFrequency()
{
    delete ui;
}

void ThreadFrequency::setScalingCurFreq(int freq)
{
    //ui->lcdNumber_CPUScalingCurFreq->display(freq);
    m_scalingCurFreq = freq;
    m_barSet->remove(0);
    m_barSet->append(freq);
    m_chart->setTitle(m_title + QString(" [%1 MHz]").arg(freq,4,10,QChar('0')));

    ui->horizontalLayout_ThreadFrequency->itemAt(0)->widget()->repaint();
    refreshToolTip();
}

void ThreadFrequency::setScalingMinFreq(int freq)
{
    m_scalingMinFreq = freq;
    refreshToolTip();
}

void ThreadFrequency::setScalingMaxFreq(int freq)
{
    m_scalingMaxFreq = freq;
    refreshToolTip();
}

void ThreadFrequency::setBaseFreq(int freq)
{
    m_baseFreq = freq;
    refreshToolTip();
}

void ThreadFrequency::setMinFreq(int freq)
{
    m_minFreq = freq;
    refreshToolTip();
}

void ThreadFrequency::setMaxFreq(int freq)
{
    m_maxFreq = freq;
    refreshToolTip();
}

void ThreadFrequency::refreshToolTip()
{
    ui->horizontalLayout_ThreadFrequency->itemAt(0)->widget()->setToolTip(
        QString("Current CPU Frequency: %1 MHz\n"
                "Scaling Min Frequency: %2 MHz\n"
                "Scaling Max Frequency: %3 MHz\n"
                "Base Frequency: %4 MHz\n"
                "Min Frequency: %5 MHz\n"
                "Max Frequency: %6 MHz")
        .arg(m_scalingCurFreq)
        .arg(m_scalingMinFreq)
        .arg(m_scalingMaxFreq)
        .arg(m_baseFreq)
        .arg(m_minFreq)
        .arg(m_maxFreq)
    );
}

}
