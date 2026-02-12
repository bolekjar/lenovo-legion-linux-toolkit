// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ThreadControl.h"
#include "ui_ThreadControl.h"

#include "MainWindow.h"

namespace LenovoLegionGui {

ThreadControl::ThreadControl(const QString& title, int index , bool enabled,const QString& scalingGovernor,quint32 coreId,const QString& color,const QString& bgcolor, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ThreadControl)
    , m_index(index)
    , m_enabled(enabled)
    , m_coreId(coreId)
    , m_color(color)
    , m_bgcolor(bgcolor)
{
    ui->setupUi(this);
    ui->groupBox->setTitle(title);
    ui->groupBox->setStyleSheet(QString("QGroupBox::title {color : %1;} QGroupBox {  background-color: %2; }").arg(m_color, m_bgcolor));
    ui->radioButton_coreEnabled->setChecked(enabled);
    ui->label_scalingGovernor_info->setText(scalingGovernor.trimmed());

    if(m_index == 0)
    {
        ui->radioButton_coreEnabled->setEnabled(false);
    }

    if(!m_enabled)
    {
        ui->groupBox->setStyleSheet(QString("QGroupBox::title {color : %1;} QGroupBox { background-color: %2;}").arg("#333066",m_bgcolor));
    }
}

ThreadControl::~ThreadControl()
{
    delete ui;
}

void ThreadControl::setCPUEnabled(bool enabled,bool currentCPUvalue)
{
    if(m_index == 0)
    {
        return;
    }

    ui->radioButton_coreEnabled->setChecked(enabled);

    if(enabled != currentCPUvalue)
    {
        ui->radioButton_coreEnabled->setStyleSheet(QString("QRadioButton {  color : %1; background-color: %2;}").arg(MainWindow::VALUE_CHANGE_COLOR.data(),m_bgcolor));
    }
    else
    {
        ui->radioButton_coreEnabled->setStyleSheet(QString("QRadioButton {  background-color: %1;}").arg(m_bgcolor));
    }
}

void ThreadControl::setScalingGovernor(const QString& governor,const QString& currentCPUGovernor)
{
    ui->label_scalingGovernor_info->setText(governor.trimmed());

    if(governor != currentCPUGovernor)
    {
        ui->label_scalingGovernor_info->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else
    {
        ui->label_scalingGovernor_info->setStyleSheet(QString("QLabel { }"));
    }
}

QString ThreadControl::getScalingGovernor() const
{
    return ui->label_scalingGovernor_info->text();
}

bool ThreadControl::getCPUEnabled() const
{
    return ui->radioButton_coreEnabled->isChecked();
}

void ThreadControl::on_radioButton_coreEnabled_toggled(bool)
{
    if(m_enabled != ui->radioButton_coreEnabled->isChecked())
    {
        ui->radioButton_coreEnabled->setStyleSheet(QString("QRadioButton {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else
    {
        ui->radioButton_coreEnabled->setStyleSheet(QString("QRadioButton {  background-color: %1;}").arg(m_bgcolor));
    }
}


}
