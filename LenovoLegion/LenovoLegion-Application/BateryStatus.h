// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once
#include "WidgetMessage.h"

#include "../LenovoLegion-PrepareBuild/Battery.pb.h"

#include <QWidget>

namespace Ui {
class BateryStatus;
}

namespace LenovoLegionGui {

class DataProvider;
class BateryStatus : public QWidget
{
    Q_OBJECT

public:
    explicit BateryStatus(DataProvider* dataProvider,QWidget *parent);

    ~BateryStatus();

    void refresh();


signals:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private:

    void renderData();

private:

    Ui::BateryStatus *ui;

    DataProvider* m_dataProvider;

    legion::messages::Battery m_bateryControlData;
};

}
