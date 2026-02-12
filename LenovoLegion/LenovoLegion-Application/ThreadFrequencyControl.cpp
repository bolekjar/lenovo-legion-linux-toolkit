// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ThreadFrequencyControl.h"
#include "ui_ThreadFrequencyControl.h"

#include "MainWindow.h"

namespace LenovoLegionGui {
ThreadFrequencyControl::ThreadFrequencyControl(const QString &title, int min, int max, int curMin, int CurMax, const QString& bgColor,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ThreadFrequencyControl)
    , m_minPrevious(curMin)
    , m_maxPrevious(CurMax)
    , m_bgColor(bgColor)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(title);

    ui->lcdNumber_CPUMinFreqScaling->display(curMin);
    ui->lcdNumber_CPUMaxFreqScaling->display(CurMax);

    ui->horizontalSlider_CPUMinFreqScaling->setMinimum(min);
    ui->horizontalSlider_CPUMinFreqScaling->setMaximum(max);
    ui->horizontalSlider_CPUMaxFreqScaling->setMinimum(min);
    ui->horizontalSlider_CPUMaxFreqScaling->setMaximum(max);

    ui->horizontalSlider_CPUMinFreqScaling->setValue(curMin);
    ui->horizontalSlider_CPUMaxFreqScaling->setValue(CurMax);

    ui->horizontalSlider_CPUMinFreqScaling->setSingleStep(1);
    ui->horizontalSlider_CPUMaxFreqScaling->setSingleStep(1);

    ui->groupBox->setStyleSheet(QString("QGroupBox { background-color: %1;}").arg(m_bgColor));
}

ThreadFrequencyControl::~ThreadFrequencyControl()
{
    delete ui;
}

unsigned int ThreadFrequencyControl::getMinFreq() const
{
    return ui->horizontalSlider_CPUMinFreqScaling->value();
}

unsigned int ThreadFrequencyControl::getMaxFreq() const
{
    return ui->horizontalSlider_CPUMaxFreqScaling->value();
}

void ThreadFrequencyControl::setMinFreq(int value, int poreviousValue)
{
    if(value < ui->horizontalSlider_CPUMinFreqScaling->minimum())
    {
        value = ui->horizontalSlider_CPUMinFreqScaling->minimum();
    }

    if(value > ui->horizontalSlider_CPUMinFreqScaling->maximum())
    {
        value = ui->horizontalSlider_CPUMinFreqScaling->maximum();
    }

    m_minPrevious = poreviousValue;
    on_horizontalSlider_CPUMinFreqScaling_valueChanged(value);
}
void ThreadFrequencyControl::setMaxFreq(int value, int poreviousValue)
{
    if(value > ui->horizontalSlider_CPUMaxFreqScaling->maximum())
    {
        value = ui->horizontalSlider_CPUMaxFreqScaling->maximum();
    }

    if(value < ui->horizontalSlider_CPUMaxFreqScaling->minimum())
    {
        value = ui->horizontalSlider_CPUMaxFreqScaling->minimum();
    }

    m_maxPrevious = poreviousValue;
    on_horizontalSlider_CPUMaxFreqScaling_valueChanged(value);
}

void ThreadFrequencyControl::on_horizontalSlider_CPUMinFreqScaling_valueChanged(int value)
{
    ui->lcdNumber_CPUMinFreqScaling->display(value);
    ui->horizontalSlider_CPUMaxFreqScaling->setValue(ui->horizontalSlider_CPUMaxFreqScaling->value() < value ? value : ui->horizontalSlider_CPUMaxFreqScaling->value());
    ui->horizontalSlider_CPUMinFreqScaling->setValue(value);

    if(m_minPrevious != value)
    {
        ui->label_CPUMinFreqScaling->setStyleSheet(QString("QLabel {  color : %1; background-color: %2;}").arg(MainWindow::VALUE_CHANGE_COLOR.data(),m_bgColor));
    }
    else
    {
        ui->label_CPUMinFreqScaling->setStyleSheet(QString("QLabel {  background-color: %1;}").arg(m_bgColor));
    }
}

void ThreadFrequencyControl::on_horizontalSlider_CPUMaxFreqScaling_valueChanged(int value)
{
    ui->lcdNumber_CPUMaxFreqScaling->display(value);
    ui->horizontalSlider_CPUMinFreqScaling->setValue(ui->horizontalSlider_CPUMinFreqScaling->value() > value ? value : ui->horizontalSlider_CPUMinFreqScaling->value());
    ui->horizontalSlider_CPUMaxFreqScaling->setValue(value);

    if(m_maxPrevious != value)
    {
        ui->label_CPUMaxFreqScaling->setStyleSheet(QString("QLabel {  color : %1; background-color: %2;}").arg(MainWindow::VALUE_CHANGE_COLOR.data(),m_bgColor));
    }
    else
    {
        ui->label_CPUMaxFreqScaling->setStyleSheet(QString("QLabel {  background-color: %1;}").arg(m_bgColor));
    }
}



}
