// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "BateryStatus.h"
#include "ui_BateryStatus.h"

#include "DataProvider.h"

#include "../LenovoLegion-PrepareBuild/Battery.pb.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderBattery.h"

namespace LenovoLegionGui {
BateryStatus::BateryStatus(DataProvider* dataProvider,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BateryStatus)
    , m_dataProvider(dataProvider)
{
    ui->setupUi(this);

    refresh();
}

BateryStatus::~BateryStatus()
{
    delete ui;
}

void BateryStatus::refresh()
{
    m_bateryControlData = m_dataProvider->getDataMessage<legion::messages::Battery>(LenovoLegionDaemon::SysFsDataProviderBattery::dataType);
    renderData();
}

void BateryStatus::renderData()
{
    if(m_bateryControlData.supported())
    {
        ui->label_bateryText->setText(QString(m_bateryControlData.baterry_status().data()).trimmed());
    }
    else
    {
        ui->label_bateryText->setText(QString("N/A"));
    }
}

}
