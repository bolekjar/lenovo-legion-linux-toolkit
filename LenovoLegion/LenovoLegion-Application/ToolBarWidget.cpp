#include "ToolBarWidget.h"
#include "DataProvider.h"

#include "Utils.h"

namespace LenovoLegionGui {



ToolBarWidget::ToolBarWidget(DataProvider *dataProvider, QWidget *parent)
    : QWidget{parent},
      m_dataProvider{dataProvider}
{}



void ToolBarWidget::mainWindowTimerEvent(QTimerEvent *)
{
    Utils::Task::executeAndClearTasks(m_asyncTasks);
}

void ToolBarWidget::dataProviderEvent(const legion::messages::Notification &)
{}

ToolBarWidget *ToolBarWidget::initialize()
{
    return this;
}

void ToolBarWidget::cleanup()
{
    m_asyncTasks.clear();
}


}
