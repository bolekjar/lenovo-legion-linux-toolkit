    // SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "HWMonitor.h"
#include "ui_HWMonitor.h"


namespace LenovoLegionGui {

HWMonitor::HWMonitor(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::HWMonitor)
    , m_chart(nullptr)
    , m_barSet(nullptr)
{
    ui->setupUi(this);
}

HWMonitor::~HWMonitor()
{
    delete ui;
}

void HWMonitor::init(const QString &title, int min, int max,int scaleMin, int scaleMax)
{
    m_title = title;
    m_minValue = min;
    m_maxValue = max;
    m_scaleMin = scaleMin;
    m_scaleMax = scaleMax;

    m_barSet = new QBarSet("");
    m_chart  = new QChart();

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

    ui->verticalLayout_2->addWidget(charView);
    ui->verticalLayout_2->setContentsMargins(0,0,0,0);
}

void HWMonitor::refresh(int value,int fieldWidth, int base, QChar fillChar,const QString& toolTip)
{
    m_value = value;
    m_barSet->remove(0);
    m_barSet->append(m_value);
    m_chart->setTitle(m_title.arg(value,fieldWidth, base, fillChar));
    m_chart->setToolTip(toolTip);
}

int HWMonitor::getMinValue() const {
    return m_minValue;
}
int HWMonitor::getMaxValue() const {
    return m_maxValue;
}

int HWMonitor::getScaleMin() const
{
    return m_scaleMin;
}

int HWMonitor::getScaleMax() const
{
    return m_scaleMax;
}

int HWMonitor::getValue() const
{
    return m_value;
}

}
