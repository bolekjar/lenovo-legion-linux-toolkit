// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QWidget>

namespace Ui {
class ThreadFrequencyControl;
}


namespace LenovoLegionGui {

class ThreadFrequencyControl : public QWidget
{
    Q_OBJECT

public:
    explicit ThreadFrequencyControl(const QString& title, int min, int max, int curMin, int CurMax, const QString &bgColor, QWidget *parent = nullptr);
    ~ThreadFrequencyControl();

    unsigned int getMinFreq() const;
    unsigned int getMaxFreq() const;

    void setMinFreq(int value,int poreviousValue);
    void setMaxFreq(int value,int poreviousValue);

private slots:

    void on_horizontalSlider_CPUMinFreqScaling_valueChanged(int value);
    void on_horizontalSlider_CPUMaxFreqScaling_valueChanged(int value);

private:
    Ui::ThreadFrequencyControl *ui;

    int m_minPrevious;
    int m_maxPrevious;
    const QString m_bgColor;
};


}
