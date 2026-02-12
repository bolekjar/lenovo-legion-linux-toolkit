// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "FanControl.h"
#include "ui_FanControl.h"

#include "DataProvider.h"

#include <MainWindow.h>


#include "../LenovoLegion-Daemon/SysFsDataProviderFanCurve.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderFanOption.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderPowerProfile.h"



namespace LenovoLegionGui {

const QString FanControl::NAME = "Fan Control";

FanControl::FanControl(DataProvider *dataProvider, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FanControl)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    refreshData();
    refresh();
}

FanControl::~FanControl()
{
    delete ui;
}

void FanControl::refresh()
{
    renderData();
}

void FanControl::on_verticalSlider_FanCurve1_valueChanged(int value)
{
    m_localFanCurveControlData.mutable_current_value()->set_point1(value);
    normalizeCurrentFanCurveControlData(0,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve2_valueChanged(int value)
{
    if(value < 2) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point2(value);
    normalizeCurrentFanCurveControlData(1,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve3_valueChanged(int value)
{
    if(value < 3) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point3(value);
    normalizeCurrentFanCurveControlData(2,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve4_valueChanged(int value)
{
    if(value < 4) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point4(value);
    normalizeCurrentFanCurveControlData(3,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve5_valueChanged(int value)
{
    if(value < 5) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point5(value);
    normalizeCurrentFanCurveControlData(4,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve6_valueChanged(int value)
{
    if(value < 6) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point6(value);
    normalizeCurrentFanCurveControlData(5,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve7_valueChanged(int value)
{
    if(value < 7) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point7(value);
    normalizeCurrentFanCurveControlData(6,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve8_valueChanged(int value)
{
    if(value < 8) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point8(value);
    normalizeCurrentFanCurveControlData(7,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve9_valueChanged(int value)
{
    if(value < 9) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }
    m_localFanCurveControlData.mutable_current_value()->set_point9(value);
    normalizeCurrentFanCurveControlData(8,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_verticalSlider_FanCurve10_valueChanged(int value)
{
    if(value < 9) {
        renderFanCurveControlData();
        markChangesFanCurveControlData();
        return;
    }

    m_localFanCurveControlData.mutable_current_value()->set_point10(value);
    normalizeCurrentFanCurveControlData(9,value);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::renderData()
{
    renderFanControlData();
    renderFanCurveControlData();
}

void FanControl::renderFanCurveControlData()
{
    ui->groupBox_fanCurve->blockSignals(true);
    ui->pushButton_FanCurveApply->blockSignals(true);
    ui->pushButton_FanCurveCancel->blockSignals(true);


    ui->groupBox_fanCurve->setEnabled(false);
    ui->pushButton_FanCurveApply->setEnabled(false);
    ui->pushButton_FanCurveCancel->setEnabled(false);


    ui->verticalSlider_FanCurve1->setValue(m_localFanCurveControlData.current_value().point1());
    ui->verticalSlider_FanCurve2->setValue(m_localFanCurveControlData.current_value().point2());
    ui->verticalSlider_FanCurve3->setValue(m_localFanCurveControlData.current_value().point3());
    ui->verticalSlider_FanCurve4->setValue(m_localFanCurveControlData.current_value().point4());
    ui->verticalSlider_FanCurve5->setValue(m_localFanCurveControlData.current_value().point5());
    ui->verticalSlider_FanCurve6->setValue(m_localFanCurveControlData.current_value().point6());
    ui->verticalSlider_FanCurve7->setValue(m_localFanCurveControlData.current_value().point7());
    ui->verticalSlider_FanCurve8->setValue(m_localFanCurveControlData.current_value().point8());
    ui->verticalSlider_FanCurve9->setValue(m_localFanCurveControlData.current_value().point9());
    ui->verticalSlider_FanCurve10->setValue(m_localFanCurveControlData.current_value().point10());


    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve1,0);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve2,1);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve3,2);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve4,3);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve5,4);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve6,5);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve7,6);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve8,7);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve9,8);
    renderToolTipsFanCurveControlData(*ui->verticalSlider_FanCurve10,9);

    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM && ui->pushButton_Custom->isChecked())
    {
        ui->groupBox_fanCurve->setEnabled(true);
        ui->pushButton_FanCurveApply->setEnabled(true);
        ui->pushButton_FanCurveCancel->setEnabled(true);

        ui->groupBox_fanCurve->blockSignals(false);
        ui->pushButton_FanCurveApply->blockSignals(false);
        ui->pushButton_FanCurveCancel->blockSignals(false);
    }
}

void FanControl::renderFanControlData()
{
    ui->pushButton_MaxSpeed->blockSignals(true);
    ui->pushButton_Custom->blockSignals(true);
    ui->pushButton_MaxSpeed->setEnabled(false);
    ui->pushButton_Custom->setEnabled(false);

    ui->pushButton_MaxSpeed->setChecked(false);
    ui->pushButton_Custom->setChecked(true);

    if(m_fanControlData.full_speed().supported() && m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        if(m_fanControlData.full_speed().current_value())
        {
            ui->pushButton_MaxSpeed->setChecked(true);
            ui->pushButton_Custom->setChecked(false);
        }
        else
        {
            ui->pushButton_Custom->setChecked(true);
            ui->pushButton_MaxSpeed->setChecked(false);
        }

        ui->pushButton_MaxSpeed->setEnabled(true);
        ui->pushButton_Custom->setEnabled(true);
        ui->pushButton_MaxSpeed->blockSignals(false);
        ui->pushButton_Custom->blockSignals(false);
    }

}

void FanControl::renderToolTipsFanCurveControlData(QSlider &slider,const int index)
{
    QString toolTip;

    /*
     * Temperatures
     */
    auto def = m_localFanCurveControlData.cpu_default().at(m_powerProfileData.current_value());
    toolTip += QString("CPU temp: %1 ℃\n").arg(def.sensors().at( index) == 127 ? QString("N/A") : QString::number(def.sensors().at( index)));


    def = m_localFanCurveControlData.gpu_default().at(m_powerProfileData.current_value());
    toolTip += QString("GPU temp: %1 ℃\n").arg(def.sensors().at( index) == 127 ? QString("N/A") : QString::number(def.sensors().at( index)));

    if(m_localFanCurveControlData.sys_default().size() > 0)
    {
        auto def = m_localFanCurveControlData.sys_default().at(m_powerProfileData.current_value());
        toolTip += QString("SYS temp: %1 ℃\n").arg(def.sensors().at( index) == 127 ? QString("N/A") : QString::number(def.sensors().at( index)));
    }

    if(m_localFanCurveControlData.cpusen_default().size() > 0)
    {
        auto def = m_localFanCurveControlData.cpusen_default().at(m_powerProfileData.current_value());
        toolTip += QString("CPU sensor temp: %1 ℃\n").arg(def.sensors().at( index) == 127 ? QString("N/A") : QString::number(def.sensors().at( index)));
    }


    /*
     * Speeds
     */
    def = m_localFanCurveControlData.cpu_default().at(m_powerProfileData.current_value());
    toolTip += QString("CPU fan speed: %1 rpm\n").arg(def.fan().at( index));

    def = m_localFanCurveControlData.gpu_default().at(m_powerProfileData.current_value());
    toolTip += QString("GPU fan speed: %1 rpm\n").arg(def.fan().at( index));

    if(m_localFanCurveControlData.sys_default().size() > 0)
    {
        def = m_localFanCurveControlData.sys_default().at(m_powerProfileData.current_value());
        toolTip += QString("SYS fan speed: %1 rpm\n").arg(def.fan().at( index));
    }

    if(m_localFanCurveControlData.cpusen_default().size() > 0)
    {
        def = m_localFanCurveControlData.cpusen_default().at(m_powerProfileData.current_value());
        toolTip += QString("CPU sensor fan speed: %1 rpm\n").arg(def.fan().at( index));
    }

    slider.setToolTip(toolTip);

}

void FanControl::markChangesFanCurveControlData()
{
    if(ui->verticalSlider_FanCurve1->value() != static_cast<int>(m_fanCurveControlData.current_value().point1()))
    {
        ui->label_FanCurve1->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve1->setStyleSheet(QString("QLabel { }"));
    }

    if(ui->verticalSlider_FanCurve2->value() != static_cast<int>(m_fanCurveControlData.current_value().point2()))
    {
        ui->label_FanCurve2->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve2->setStyleSheet(QString("QLabel { }"));
    }

    if(ui->verticalSlider_FanCurve3->value() != static_cast<int>(m_fanCurveControlData.current_value().point3()))
    {
        ui->label_FanCurve3->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve3->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve4->value() != static_cast<int>(m_fanCurveControlData.current_value().point4()))
    {
        ui->label_FanCurve4->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve4->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve5->value() != static_cast<int>(m_fanCurveControlData.current_value().point5()))
    {
        ui->label_FanCurve5->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve5->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve6->value() != static_cast<int>(m_fanCurveControlData.current_value().point6()))
    {
        ui->label_FanCurve6->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve6->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve7->value() != static_cast<int>(m_fanCurveControlData.current_value().point7()))
    {
        ui->label_FanCurve7->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve7->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve8->value() != static_cast<int>(m_fanCurveControlData.current_value().point8()))
    {
        ui->label_FanCurve8->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve8->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve9->value() != static_cast<int>(m_fanCurveControlData.current_value().point9()))
    {
        ui->label_FanCurve9->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve9->setStyleSheet(QString("QLabel { }"));
    }
    if(ui->verticalSlider_FanCurve10->value() != static_cast<int>(m_fanCurveControlData.current_value().point10()))
    {
        ui->label_FanCurve10->setStyleSheet(QString("QLabel {  color : ").append(MainWindow::VALUE_CHANGE_COLOR).append("; }"));
    }
    else {
        ui->label_FanCurve10->setStyleSheet(QString("QLabel { }"));
    }
}

void FanControl::normalizeCurrentFanCurveControlData(int fromIndex, int value)
{
    std::array<quint32,10> points = {  m_localFanCurveControlData.current_value().point1(),
                                       m_localFanCurveControlData.current_value().point2(),
                                       m_localFanCurveControlData.current_value().point3(),
                                       m_localFanCurveControlData.current_value().point4(),
                                       m_localFanCurveControlData.current_value().point5(),
                                       m_localFanCurveControlData.current_value().point6(),
                                       m_localFanCurveControlData.current_value().point7(),
                                       m_localFanCurveControlData.current_value().point8(),
                                       m_localFanCurveControlData.current_value().point9(),
                                       m_localFanCurveControlData.current_value().point10()};

    for (size_t var = fromIndex; var < points.size(); ++var) {
        if(points.at(var) < static_cast<quint32>(value))
        {
            points.at(var) = value;
        }
    }

    for (int var = std::min(static_cast<int>(points.size()),fromIndex); var > -1; --var) {
        if(points.at(var) > static_cast<quint32>(value))
        {
            points.at(var) = value;
        }
    }

    m_localFanCurveControlData.mutable_current_value()->set_point1(points.at(0));
    m_localFanCurveControlData.mutable_current_value()->set_point2(points.at(1));
    m_localFanCurveControlData.mutable_current_value()->set_point3(points.at(2));
    m_localFanCurveControlData.mutable_current_value()->set_point4(points.at(3));
    m_localFanCurveControlData.mutable_current_value()->set_point5(points.at(4));
    m_localFanCurveControlData.mutable_current_value()->set_point6(points.at(5));
    m_localFanCurveControlData.mutable_current_value()->set_point7(points.at(6));
    m_localFanCurveControlData.mutable_current_value()->set_point8(points.at(7));
    m_localFanCurveControlData.mutable_current_value()->set_point9(points.at(8));
    m_localFanCurveControlData.mutable_current_value()->set_point10(points.at(9));
}

void FanControl::on_pushButton_FanCurveApply_clicked()
{
    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderFanCurve::dataType,m_localFanCurveControlData);
    m_localFanCurveControlData = m_fanCurveControlData = m_dataProvider->getDataMessage<legion::messages::FanCurve>(LenovoLegionDaemon::SysFsDataProviderFanCurve::dataType);
    renderFanCurveControlData();
    markChangesFanCurveControlData();
}

void FanControl::on_pushButton_FanCurveCancel_clicked()
{
    m_localFanCurveControlData = m_fanCurveControlData;
    renderFanCurveControlData();
}

void FanControl::on_pushButton_MaxSpeed_clicked()
{
    legion::messages::FanOption data;

    data.mutable_full_speed()->set_current_value(true);

    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderFanOption::dataType,data);
    refreshData();
    refresh();
}

void FanControl::on_pushButton_Custom_clicked()
{
    legion::messages::FanOption data;

    data.mutable_full_speed()->set_current_value(false);

    m_dataProvider->setDataMessage(LenovoLegionDaemon::SysFsDataProviderFanOption::dataType,data);
    refreshData();
    refresh();
}

void FanControl::refreshData()
{
    m_fanControlData           = m_dataProvider->getDataMessage<legion::messages::FanOption>(LenovoLegionDaemon::SysFsDataProviderFanOption::dataType);
    m_powerProfileData         = m_dataProvider->getDataMessage<legion::messages::PowerProfile>(LenovoLegionDaemon::SysFsDataProviderPowerProfile::dataType);
    m_fanCurveControlData      = m_dataProvider->getDataMessage<legion::messages::FanCurve>(LenovoLegionDaemon::SysFsDataProviderFanCurve::dataType);


    if(!m_fanControlData.has_full_speed()           ||
       !m_powerProfileData.has_thermal_mode()       ||
       !m_fanCurveControlData.has_current_value()
       )
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DATA_NOT_READY,"Fan control data not ready");
    }


    if(m_powerProfileData.thermal_mode() == legion::messages::PowerProfile::POWER_PROFILE_CUSTOM)
    {
        if(m_fanControlData.full_speed().supported() && m_fanControlData.full_speed().current_value())
        {
            m_fanCurveControlData.mutable_current_value()->set_point1(10);
            m_fanCurveControlData.mutable_current_value()->set_point2(10);
            m_fanCurveControlData.mutable_current_value()->set_point3(10);
            m_fanCurveControlData.mutable_current_value()->set_point4(10);
            m_fanCurveControlData.mutable_current_value()->set_point5(10);
            m_fanCurveControlData.mutable_current_value()->set_point6(10);
            m_fanCurveControlData.mutable_current_value()->set_point7(10);
            m_fanCurveControlData.mutable_current_value()->set_point8(10);
            m_fanCurveControlData.mutable_current_value()->set_point9(10);
            m_fanCurveControlData.mutable_current_value()->set_point10(10);
        }
    }
    else
    {
        m_fanCurveControlData.mutable_current_value()->set_point1(1);
        m_fanCurveControlData.mutable_current_value()->set_point2(2);
        m_fanCurveControlData.mutable_current_value()->set_point3(3);
        m_fanCurveControlData.mutable_current_value()->set_point4(4);
        m_fanCurveControlData.mutable_current_value()->set_point5(5);
        m_fanCurveControlData.mutable_current_value()->set_point6(6);
        m_fanCurveControlData.mutable_current_value()->set_point7(7);
        m_fanCurveControlData.mutable_current_value()->set_point8(8);
        m_fanCurveControlData.mutable_current_value()->set_point9(9);
        m_fanCurveControlData.mutable_current_value()->set_point10(10);
    }


    m_localFanCurveControlData = m_fanCurveControlData;
}


}
