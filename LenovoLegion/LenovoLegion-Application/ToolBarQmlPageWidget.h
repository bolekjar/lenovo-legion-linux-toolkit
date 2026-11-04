// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "ToolBarWidget.h"

#include <QUrl>

class QQuickWidget;

namespace LenovoLegionGui {

/*
 * Page of the generic GUI (UiDesign.md): QML page in a QQuickWidget as one toolbar page.
 * Daemon notifications reload all data sources of the page.
 */
class ToolBarQmlPageWidget : public ToolBarWidget
{
    Q_OBJECT

public:

    ToolBarQmlPageWidget(DataProvider* dataProvider,const QUrl& page,QWidget *parent = nullptr);
    ~ToolBarQmlPageWidget() override;

    virtual void dataProviderEvent(const legion::messages::Notification &event) override;

private:

    /*
     * QML types of the generic GUI (once per application)
     */
    static void registerTypes();

private:

    QQuickWidget* m_view;
};

}
