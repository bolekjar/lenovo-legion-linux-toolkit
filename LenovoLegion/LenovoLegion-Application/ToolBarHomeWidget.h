// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ToolBarWidget.h"


namespace Ui {
class ToolBarHomeWidget;
}


namespace LenovoLegionGui {


class WidgetMessage;

class ToolBarHomeWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    explicit ToolBarHomeWidget(DataProvider* dataProvider,QWidget *parent = nullptr);

    ~ToolBarHomeWidget();

    virtual void mainWindowTimerEvent(QTimerEvent *) override;
    virtual void dataProviderEvent(const legion::messages::Notification &event) override;

    virtual void cleanup() override;

protected slots:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private:

    Ui::ToolBarHomeWidget *ui;


    /*
     * Default Action Map, Defined by user
     */
    std::map<std::string,std::list<std::function< void ()>>> m_defaultActionsMap;


    int m_timerId = -1;
};

}
