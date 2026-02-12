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
class ToolBarKeyboardWidget;
}


namespace LenovoLegionGui {


class WidgetMessage;

class ToolBarKeyboardWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    explicit ToolBarKeyboardWidget(DataProvider* dataProvider,QWidget *parent = nullptr);

    ~ToolBarKeyboardWidget();


    virtual void dataProviderEvent(const legion::messages::Notification &event) override;
    virtual void cleanup() override;


protected slots:

    void widgetEvent(const LenovoLegionGui::WidgetMessage& event);

private:

    Ui::ToolBarKeyboardWidget *ui;

    /*
     * Default Action Map, Defined by user
     */
    std::map<std::string,std::list<std::function< void ()>>> m_defaultActionsMap;
};

}
