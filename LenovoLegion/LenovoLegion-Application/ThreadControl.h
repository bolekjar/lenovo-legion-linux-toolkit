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
class ThreadControl;
}

namespace LenovoLegionGui {

class ThreadControl : public QWidget
{
    Q_OBJECT

public:

    explicit ThreadControl(const QString& title,
                           int index,
                           bool    enabled,
                           const QString &scalingGovernor,
                           quint32 coreId,
                           const QString &color, const QString &bgcolor,
                           QWidget *parent = nullptr
                           );
    ~ThreadControl();

    void setCPUEnabled(bool enabled,bool currentCPUValue);
    void setScalingGovernor(const QString& governor,const QString& currentCPUGovernor);

    QString getScalingGovernor() const;
    bool getCPUEnabled() const;

private slots:
    void on_radioButton_coreEnabled_toggled(bool checked);

private:
    Ui::ThreadControl *ui;
    const int   m_index;
    const bool  m_enabled;
    const int   m_coreId;
    const QString m_color;
    const QString m_bgcolor;
};

}
