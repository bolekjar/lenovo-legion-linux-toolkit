// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QWidget>



#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"


namespace Ui {
class GPUDetails;
}


namespace LenovoLegionGui {

class GPUDetails : public QWidget
{
    Q_OBJECT

public:

    explicit GPUDetails(QWidget *parent = nullptr);

    void init(const legion::messages::NvidiaNvml &nvidiaData);
    void refresh(const legion::messages::NvidiaNvml &nvidiaData);

    ~GPUDetails();


    void closeEvent(QCloseEvent *event) override;

signals:

    void closed();

private:

    quint64 calculateTheoreticalMaxPcieThroughput(const legion::messages::NvidiaNvml &nvidiaData);

private:
    Ui::GPUDetails *ui;

};

}
