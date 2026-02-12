// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "DataProvider.h"
#include <QWidget>


#include "../LenovoLegion-PrepareBuild/Notification.pb.h"

namespace LenovoLegionGui {



class ToolBarWidget : public QWidget
{
    Q_OBJECT

public:

    explicit ToolBarWidget(DataProvider* dataProvider,QWidget *parent);

    virtual ~ToolBarWidget() = default;


    virtual ToolBarWidget* initialize();                //Initialize after construction


    virtual void cleanup();                             //Cleanup before destruction or removal from UI
    virtual void mainWindowTimerEvent(QTimerEvent *);   //Timer event from MainWindow delegation

    virtual void dataProviderEvent(const legion::messages::Notification &);
signals:

protected:

    DataProvider* const m_dataProvider;

    /*
     * Actions for evaluation
     */
    std::list<std::function< void ()>>                       m_asyncTasks;

};


}
