// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QWidget>
#include <QtCharts/QtCharts>

namespace Ui {
class HWMonitor;
}


namespace LenovoLegionGui {

class HWMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit HWMonitor(QWidget *parent = nullptr);
    ~HWMonitor();

    int getMinValue() const;
    int getMaxValue() const;
    int getScaleMin() const;
    int getScaleMax() const;
    int getValue() const;

    void init(const QString& title,int min,int max,int scaleMin, int scaleMax);
    void refresh(int value,int fieldWidth = 0, int base = 10,QChar fillChar = u' ',const QString& toolTip = "");

private:
    Ui::HWMonitor *ui;
    QChart  *m_chart;
    QBarSet *m_barSet;
    QString   m_title;
    int      m_minValue = 0;
    int      m_maxValue = 100;
    int      m_scaleMin = 0;
    int      m_scaleMax = 100;
    int      m_value = 0;
};

}
