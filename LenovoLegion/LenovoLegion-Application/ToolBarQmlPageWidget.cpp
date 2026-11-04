// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "ToolBarQmlPageWidget.h"

#include "Ui/DataSource.h"
#include "Ui/FieldModel.h"
#include "Ui/PageSources.h"
#include "Ui/Theme.h"

#include "Settings.h"

#include <Core/LoggerHolder.h>

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickStyle>
#include <QQuickWidget>
#include <QVBoxLayout>

namespace LenovoLegionGui {

ToolBarQmlPageWidget::ToolBarQmlPageWidget(DataProvider *dataProvider, const QUrl &page, QWidget *parent) :
    ToolBarWidget(dataProvider,parent),
    m_view(nullptr)
{
    registerTypes();

    DataSource::setDataProvider(m_dataProvider);

    m_view = new QQuickWidget(this);
    m_view->setResizeMode(QQuickWidget::SizeRootObjectToView);

    /*
     * Theme of the application, follows the theme selected in Settings
     */
    ApplicationSettings::ThemeType themeType = ApplicationSettings::ThemeType::NoTheme;
    ApplicationSettings::instance()->loadStylesheetTheme(themeType);

    auto* theme = new Theme(themeType,m_view);

    connect(ApplicationSettings::instance(),&ApplicationSettings::themeChanged,theme,&Theme::setTheme);

    m_view->rootContext()->setContextProperty("theme",theme);

    connect(m_view,&QQuickWidget::statusChanged,this,[this](QQuickWidget::Status status) {
        if(status == QQuickWidget::Error)
        {
            for(const auto& error : m_view->errors())
            {
                LOG_E(QString("QML page: ").append(error.toString()));
            }
        }
    });

    m_view->setSource(page);

    auto* layout = new QVBoxLayout(this);

    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(m_view);
}

ToolBarQmlPageWidget::~ToolBarQmlPageWidget()
{
    /*
     * Data sources of the page are deleted with the view
     */
    delete m_view;
}

void ToolBarQmlPageWidget::dataProviderEvent(const legion::messages::Notification &)
{
    DataSource::reloadAll();
}

void ToolBarQmlPageWidget::registerTypes()
{
    static bool registered = false;

    if(registered)
    {
        return;
    }

    registered = true;

    QQuickStyle::setStyle("Fusion");

    qmlRegisterType<DataSource>("LenovoLegion.Ui",1,0,"DataSource");
    qmlRegisterType<PageSources>("LenovoLegion.Ui",1,0,"PageSources");
    qmlRegisterUncreatableType<FieldModel>("LenovoLegion.Ui",1,0,"FieldModel","FieldModel is created by DataSource.field()");
}

}
