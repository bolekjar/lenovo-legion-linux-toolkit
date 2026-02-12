#include "ToolBarHomeWidget.h"
#include "Core/LoggerHolder.h"
#include "ui_ToolBarHomeWidget.h"

#include "Utils.h"

#include "CPUControl.h"
#include "CPUFrequencyControl.h"
#include "HWMonitoring.h"
#include "PowerProfileControl.h"
#include "FanControl.h"
#include "PowerControl.h"
#include "OffsetsControl.h"
#include "OtherControl.h"



namespace LenovoLegionGui {


namespace  {

template<class T>
inline static void addTabWidgetControls(T *widget, QTabWidget &tabWidget,const QString& name,std::function<void (const WidgetMessage&)> signalHandler)
{
    tabWidget.addTab(widget,name);
    QObject::connect(widget,&T::widgetEvent,signalHandler);
    widget->refresh();
}

template<class T>
inline static void addLayoutWidget(T *widget, QBoxLayout &layout,std::function<void (const WidgetMessage&)> signalHandler)
{

    layout.insertWidget(0,widget);
    QObject::connect(widget,&T::widgetEvent,signalHandler);
    widget->refresh();
}


void removeControlTab(const QString &name, QTabWidget &tabWidget) {

    /*
     * Remove tab
     */
    for(int i = 0; i < tabWidget.count();)
    {
        if(tabWidget.tabText(i) == name)
        {
            tabWidget.widget(i)->deleteLater();
            tabWidget.removeTab(i);
            i = 0;
        }
        else
        {
            i++;
        }
    }
}

void removeAllControlTab(QTabWidget &tabWidget) {
    /*
     * Remove tab
     */
    while(tabWidget.count() > 0)
    {
        tabWidget.widget(0)->deleteLater();
        tabWidget.removeTab(0);
    }
}

template<class T>
void refreshControlTabByName(const QString &name, QTabWidget &tabWidget)
{
    /*
     * Refresh tab
     */
    for(int i = 0; i < tabWidget.count();++i)
    {
        if(tabWidget.tabText(i) == name)
        {
            dynamic_cast<T*>(tabWidget.widget(i))->refresh();
        }
    }
}

}


ToolBarHomeWidget::ToolBarHomeWidget(DataProvider* dataProvider,QWidget *parent)
    : ToolBarWidget(dataProvider,parent)
    , ui(new Ui::ToolBarHomeWidget)
{

    ui->setupUi(this);

    /*
     * Initialize actions map
     */

    m_defaultActionsMap["add"].push_back([this]() {
        try {
            addTabWidgetControls(new CPUControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,CPUControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
            addTabWidgetControls(new CPUFrequencyControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,CPUFrequencyControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));

            addLayoutWidget(new HWMonitoring(m_dataProvider,this),*ui->horizontalLayout_HWMonitoring,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
            addLayoutWidget(new PowerProfileControl(m_dataProvider,this),*ui->verticalLayout_PowerProfiles,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
        } catch(CPUControl::exception_T& ex)
        {
            LOG_W(QString("CPUControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
        catch(CPUFrequencyControl::exception_T& ex)
        {
            LOG_W(QString("CPUFrequencyControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
    });


    m_defaultActionsMap["remove"].push_back([this]() {
        removeAllControlTab(*ui->tabWidget_Controls);

        Utils::Layout::removeAllLayoutWidgets(*ui->horizontalLayout_HWMonitoring);
        Utils::Layout::removeAllLayoutWidgets(*ui->verticalLayout_PowerProfiles);
    });

    m_defaultActionsMap["add_customs"].push_back([this]() {
        try {
            addTabWidgetControls(new PowerControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,PowerControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
            addTabWidgetControls(new FanControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,FanControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
            addTabWidgetControls(new OffsetsControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,OffsetsControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
            addTabWidgetControls(new OtherControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,OtherControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
        } catch(PowerControl::exception_T& ex)
        {
            LOG_W(QString("PowerControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
        catch(FanControl::exception_T& ex)
        {
            LOG_W(QString("FanControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
        catch (OffsetsControl::exception_T& ex)
        {
            LOG_W(QString("VoltageControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
        catch (OtherControl::exception_T& ex)
        {
            LOG_W(QString("OtherControl not added to ToolBarHomeWidget: ").append(ex.what()));
        }
    });

    m_defaultActionsMap["remove_customs"].push_back([this]() {
        removeControlTab(PowerControl::NAME,*ui->tabWidget_Controls);
        removeControlTab(FanControl::NAME,*ui->tabWidget_Controls);
        removeControlTab(OffsetsControl::NAME,*ui->tabWidget_Controls);
        removeControlTab(OtherControl::NAME,*ui->tabWidget_Controls);
    });

    m_defaultActionsMap["refresh_customs"] .push_back([this]() {
        Utils::Task::executeTasks(m_defaultActionsMap["remove_customs"]);
        Utils::Task::executeTasks(m_defaultActionsMap["add_customs"]);
    });

    m_defaultActionsMap["remove_cpu_controls"] .push_back([this]() {
        removeControlTab(CPUControl::NAME,*ui->tabWidget_Controls);
        removeControlTab(CPUFrequencyControl::NAME,*ui->tabWidget_Controls);
    });

    m_defaultActionsMap["add_cpu_controls"] .push_back([this]() {
        addTabWidgetControls(new CPUControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,CPUControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
        addTabWidgetControls(new CPUFrequencyControl(m_dataProvider,ui->tabWidget_Controls),*ui->tabWidget_Controls,CPUFrequencyControl::NAME,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
    });


    m_defaultActionsMap["refresh_cpu_controls"] .push_back([this]() {
        Utils::Task::executeTasks(m_defaultActionsMap["remove_cpu_controls"]);
        Utils::Task::executeTasks(m_defaultActionsMap["add_cpu_controls"]);
    });

    m_defaultActionsMap["refresh_hw_monitoring"] .push_back([this]() {
        Utils::Layout::removeAllLayoutWidgets(*ui->horizontalLayout_HWMonitoring);

        addLayoutWidget(new HWMonitoring(m_dataProvider,this),*ui->horizontalLayout_HWMonitoring,std::bind(&ToolBarHomeWidget::widgetEvent,this,std::placeholders::_1));
    });

    Utils::Task::insertTasksBack(m_asyncTasks,m_defaultActionsMap["add"]);
}

void ToolBarHomeWidget::mainWindowTimerEvent(QTimerEvent *event)
{
    ToolBarWidget::mainWindowTimerEvent(event);
}

void ToolBarHomeWidget::dataProviderEvent(const legion::messages::Notification &event)
{
    switch (event.action()) {
        case legion::messages::Notification::LENOVO_DRIVER_ADDED:
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_hw_monitoring"]);
            Utils::Layout::forAllLayoutsDo(*ui->verticalLayout_PowerProfiles,[](QLayoutItem &item){
                dynamic_cast<PowerProfileControl*>(item.widget())->refresh();
            });
        }
        break;
        case legion::messages::Notification::LENOVO_DRIVER_REMOVED:
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_hw_monitoring"]);
            Utils::Layout::forAllLayoutsDo(*ui->verticalLayout_PowerProfiles,[](QLayoutItem &item){
                dynamic_cast<PowerProfileControl*>(item.widget())->refresh();
            });
        }
        break;
        case legion::messages::Notification::CPU_X_LIST_RELOADED:
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_cpu_controls"]);
            Utils::Layout::forAllLayoutsDo(*ui->verticalLayout_PowerProfiles,[](QLayoutItem &item){
                dynamic_cast<PowerProfileControl*>(item.widget())->refresh();
            });
        }
        break;
        case legion::messages::Notification::THERMAL_MODE_CHANGE:
        {
            Utils::Layout::forAllLayoutsDo(*ui->verticalLayout_PowerProfiles,[](QLayoutItem &item){
                dynamic_cast<PowerProfileControl*>(item.widget())->refresh();
            });
        }
        break;
        case legion::messages::Notification::KEYLOCK_STATUS_CHANGE:
        {
            refreshControlTabByName<OtherControl>(OtherControl::NAME,*ui->tabWidget_Controls);
        }
        break;
        default:
        break;
    }
}

void ToolBarHomeWidget::cleanup()
{
    ToolBarWidget::cleanup();
}

ToolBarHomeWidget::~ToolBarHomeWidget()
{
    delete ui;
}

void ToolBarHomeWidget::widgetEvent(const WidgetMessage &event)
{
    if(event.m_widget == WidgetMessage::Widget::POWER_PROFILE_CONTROL)
    {
        if(event.m_message == WidgetMessage::Message::POWER_PROFILE_CHANGED_CUSTOM ||
           event.m_message == WidgetMessage::Message::POWER_PROFILE_CHANGED)
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_customs"]);
        }

        if(event.m_message == WidgetMessage::Message::POWER_PROFILE_NOT_AVAILABLE)
        {
            Utils::Task::executeTasks(m_defaultActionsMap["remove_customs"]);
        }
    }

    if(event.m_widget == WidgetMessage::Widget::CPU_CONTROL)
    {
        if(event.m_message == WidgetMessage::Message::CPU_CONTROL_CHANGED)
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_hw_monitoring"]);
            Utils::Task::executeTasks(m_defaultActionsMap["refresh_cpu_controls"]);

            Utils::Layout::forAllLayoutsDo(*ui->verticalLayout_PowerProfiles,[](QLayoutItem &item){
                dynamic_cast<PowerProfileControl*>(item.widget())->refresh();
            });

        }

        if(event.m_message == WidgetMessage::Message::CPU_CONTROL_NOT_AVAILABLE)
        {
            Utils::Task::executeTasks(m_defaultActionsMap["remove_cpu_controls"]);
        }
    }

    if(event.m_widget == WidgetMessage::Widget::CPU_FREQUENCY_CONTROL)
    {
        if(event.m_message == WidgetMessage::Message::CPU_FREQ_CONTROL_APPLY)
        {
            //no action
        }

        if(event.m_message == WidgetMessage::Message::CPU_FREQ_CONTROL_NOT_AVAILABLE)
        {
            Utils::Task::executeTasks(m_defaultActionsMap["remove_cpu_controls"]);
        }
    }
}


}
