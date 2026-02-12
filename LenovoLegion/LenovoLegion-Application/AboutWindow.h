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
class AboutWindow;
}

namespace LenovoLegionGui {

class AboutWindow : public QWidget
{
    Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);
    ~AboutWindow();

protected:

    void closeEvent(QCloseEvent *event) override;

private:
    Ui::AboutWindow *ui;
};

}
