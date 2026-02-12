// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QWidget>
#include "WidgetMessage.h"

#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-PrepareBuild/Other.pb.h"


namespace Ui {
class OtherControl;
}


namespace LenovoLegionGui {

class DataProvider;
class OtherControl : public QWidget
{
    Q_OBJECT

public:
    DEFINE_EXCEPTION(OtherControl)

    enum ERROR_CODES : int {
        DATA_NOT_READY = -1
    };


    static const QString NAME;

public:
    explicit OtherControl(DataProvider *dataProvider ,QWidget *parent = nullptr);
    ~OtherControl();

    void refresh();

signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private slots:
    void on_checkBox_DisableTouchpad_stateChanged(int state);
    void on_checkBox_DisableWinKey_stateChanged(int state);
    void on_radioButton_HybridOn_clicked();
    void on_radioButton_HybridAuto_clicked();
    void on_radioButton_HybridIGPUOnly_clicked();
    void on_radioButton_HybridOff_clicked();

private:

    void renderData();
    void renderGpuSwitchData();
    void applySettings();
    void applyGpuSwitchSettings();
    bool isRestartNeededForGpuSwitch(const legion::messages::GpuSwitchValue &newSettings);

private:
    Ui::OtherControl *ui;

    legion::messages::OtherSettings m_otherSettingsData;
    legion::messages::GpuSwitchValue m_gpuSwitchData;

    DataProvider *m_dataProvider;
};

}
