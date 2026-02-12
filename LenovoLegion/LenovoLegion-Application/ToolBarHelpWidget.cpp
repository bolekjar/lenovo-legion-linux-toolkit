#include "ToolBarHelpWidget.h"
#include "ui_ToolBarHelpWidget.h"

#include "Utils.h"
#include "WidgetMessage.h"


namespace LenovoLegionGui {


ToolBarHelpWidget::ToolBarHelpWidget(DataProvider* dataProvider,QWidget *parent)
    : ToolBarWidget(dataProvider,parent)
    , ui(new Ui::ToolBarHelpWidget)
{

    ui->setupUi(this);

    /*
     * Initialize actions map
     */

    m_defaultActionsMap["add"].push_back([]() {
        // Help content is static, nothing to load
    });


    m_defaultActionsMap["remove"].push_back([]() {
        // Help content is static, nothing to clear
    });

    m_defaultActionsMap["refresh"].push_back([]() {
        // Help content is static, nothing to refresh
    });

    Utils::Task::insertTasksBack(m_asyncTasks,m_defaultActionsMap["add"]);
}

void ToolBarHelpWidget::dataProviderEvent(const legion::messages::Notification &)
{}

void ToolBarHelpWidget::cleanup()
{
    ToolBarWidget::cleanup();
}

ToolBarHelpWidget::~ToolBarHelpWidget()
{
    delete ui;
}

void ToolBarHelpWidget::widgetEvent(const WidgetMessage &event)
{
    // Handle widget events if needed
    Q_UNUSED(event);
}


}
