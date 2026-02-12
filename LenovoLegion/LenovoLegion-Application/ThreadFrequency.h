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
class ThreadFrequency;
}


namespace LenovoLegionGui {

class ThreadFrequency : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadFrequency(const QString& title,int min,int max,QWidget *parent = nullptr);
    ~ThreadFrequency();

    void setScalingCurFreq(int freq);
    void setScalingMinFreq(int freq);
    void setScalingMaxFreq(int freq);

    void setBaseFreq(int freq);
    void setMinFreq(int freq);
    void setMaxFreq(int freq);

private:

    void refreshToolTip();

private:
    Ui::ThreadFrequency *ui;
    QChart  *m_chart;
    QBarSet *m_barSet;

    int m_scalingCurFreq = 0;
    int m_scalingMinFreq = 0;
    int m_scalingMaxFreq = 0;
    int m_baseFreq = 0;
    int m_minFreq  = 0;
    int m_maxFreq  = 0;

public:
    const QString m_title;
};

}
