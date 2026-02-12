// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "WidgetMessage.h"

#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-PrepareBuild/FanControl.pb.h"
#include "../LenovoLegion-PrepareBuild/PowerProfile.pb.h"

#include <QWidget>

namespace Ui {
class FanControl;
}

class QSlider;
namespace LenovoLegionGui {

class DataProvider;
class FanControl : public QWidget
{
    Q_OBJECT

public:

    DEFINE_EXCEPTION(FanControl)

    enum ERROR_CODES : int {
        DATA_NOT_READY = -1
    };

public:

    static const QString              NAME;

public:
    explicit FanControl(DataProvider *dataProvider,QWidget *parent = nullptr);
    ~FanControl();

    void refresh();

signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private slots:


    //void on_checkBox_MaxFanSpeed_checkStateChanged(const Qt::CheckState &arg1);

    void on_verticalSlider_FanCurve1_valueChanged(int value);
    void on_verticalSlider_FanCurve2_valueChanged(int value);
    void on_verticalSlider_FanCurve3_valueChanged(int value);
    void on_verticalSlider_FanCurve4_valueChanged(int value);
    void on_verticalSlider_FanCurve5_valueChanged(int value);
    void on_verticalSlider_FanCurve6_valueChanged(int value);
    void on_verticalSlider_FanCurve7_valueChanged(int value);
    void on_verticalSlider_FanCurve8_valueChanged(int value);
    void on_verticalSlider_FanCurve9_valueChanged(int value);
    void on_verticalSlider_FanCurve10_valueChanged(int value);

    void on_pushButton_FanCurveApply_clicked();
    void on_pushButton_FanCurveCancel_clicked();

    void on_pushButton_MaxSpeed_clicked();

    void on_pushButton_Custom_clicked();

private:

    void refreshData();
    void renderData();
    void renderFanCurveControlData();
    void renderFanControlData();
    void renderToolTipsFanCurveControlData(QSlider &slider, const int index);

    void markChangesFanCurveControlData();

    void normalizeCurrentFanCurveControlData(int fromIndex,int value);
private:
    Ui::FanControl *ui;

    DataProvider                          *m_dataProvider;

    legion::messages::FanOption            m_fanControlData;
    legion::messages::FanCurve             m_fanCurveControlData,
                                           m_localFanCurveControlData;
    legion::messages::PowerProfile         m_powerProfileData;

};

}
