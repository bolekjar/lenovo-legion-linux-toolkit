#include "ToolBarInformationWidget.h"
#include "Core/LoggerHolder.h"
#include "ui_ToolBarInformationWidget.h"

#include "Utils.h"
#include "WidgetMessage.h"

#include "../LenovoLegion-Daemon/SysFsDataProviderMachineInformation.h"
#include "../LenovoLegion-Daemon/SysFsDataProviderCPUInfo.h"
#include "../LenovoLegion-Daemon/DataProviderNvidiaNvml.h"
#include "../LenovoLegion-PrepareBuild/ComputerInfo.pb.h"
#include "../LenovoLegion-PrepareBuild/NvidiaNvml.pb.h"


namespace LenovoLegionGui {


ToolBarInformationWidget::ToolBarInformationWidget(DataProvider* dataProvider,QWidget *parent)
    : ToolBarWidget(dataProvider,parent)
    , ui(new Ui::ToolBarInformationWidget)
{

    ui->setupUi(this);

    /*
     * Initialize actions map
     */

    m_defaultActionsMap["add"].push_back([this]() {
        try {
            loadMachineInformation();
            displayMachineInformation();
        } catch(std::exception& ex)
        {
            LOG_W(QString("Machine Information not loaded: ").append(ex.what()));
        }
        
        try {
            loadCPUInformation();
            displayCPUInformation();
        } catch(std::exception& ex)
        {
            LOG_W(QString("CPU Information not loaded: ").append(ex.what()));
        }
        
        try {
            loadGPUInformation();
            displayGPUInformation();
        } catch(std::exception& ex)
        {
            LOG_W(QString("GPU Information not loaded: ").append(ex.what()));
        }
    });


    m_defaultActionsMap["remove"].push_back([this]() {
        // Clear all information labels
        ui->label_BiosDateValue->clear();
        ui->label_BiosReleaseValue->clear();
        ui->label_BiosVendorValue->clear();
        ui->label_BiosVersionValue->clear();
        ui->label_BoardNameValue->clear();
        ui->label_BoardVendorValue->clear();
        ui->label_BoardVersionValue->clear();
        ui->label_ChassisTypeValue->clear();
        ui->label_ChassisVendorValue->clear();
        ui->label_ChassisVersionValue->clear();
        ui->label_ProductFamilyValue->clear();
        ui->label_ProductNameValue->clear();
        ui->label_ProductSkuValue->clear();
        ui->label_ProductVersionValue->clear();
        ui->label_SysVendorValue->clear();
        ui->label_CPUModelValue->clear();
        ui->label_CPUGenerationValue->clear();
        ui->label_CPUArchitectureValue->clear();
        ui->label_CPUCacheValue->clear();
        ui->label_GPUNameValue->clear();
        ui->label_GPUPowerLimitValue->clear();
    });

    m_defaultActionsMap["refresh"].push_back([this]() {
        Utils::Task::executeTasks(m_defaultActionsMap["remove"]);
        Utils::Task::executeTasks(m_defaultActionsMap["add"]);
    });

    Utils::Task::insertTasksBack(m_asyncTasks,m_defaultActionsMap["add"]);
}

void ToolBarInformationWidget::loadMachineInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    try {
        m_machineInfo = m_dataProvider->getDataMessage<legion::messages::MachineInformation>(
            LenovoLegionDaemon::SysFsDataProviderMachineInformation::dataType);
    } catch(DataProvider::exception_T& ex)
    {
        LOG_W(QString("Failed to load machine information: ").append(ex.what()));
        throw;
    }
}

void ToolBarInformationWidget::displayMachineInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    // Display BIOS information
    ui->label_BiosDateValue->setText(m_machineInfo.bios_date().data());
    ui->label_BiosReleaseValue->setText(m_machineInfo.bios_release().data());
    ui->label_BiosVendorValue->setText(m_machineInfo.bios_vendor().data());
    ui->label_BiosVersionValue->setText((m_machineInfo.bios_version().data()));
    
    // Display Board information
    ui->label_BoardNameValue->setText(m_machineInfo.board_name().data());
    ui->label_BoardVendorValue->setText(m_machineInfo.board_vendor().data());
    ui->label_BoardVersionValue->setText(m_machineInfo.board_version().data());
    
    // Display Chassis information
    ui->label_ChassisTypeValue->setText(m_machineInfo.chassis_type().data());
    ui->label_ChassisVendorValue->setText(m_machineInfo.chassis_vendor().data());
    ui->label_ChassisVersionValue->setText(m_machineInfo.chassis_version().data());
    
    // Display Product information
    ui->label_ProductFamilyValue->setText(m_machineInfo.product_family().data());
    ui->label_ProductNameValue->setText(m_machineInfo.product_name().data());
    ui->label_ProductSkuValue->setText(m_machineInfo.product_sku().data());
    ui->label_ProductVersionValue->setText(m_machineInfo.product_version().data());
    
    // Display System Vendor
    ui->label_SysVendorValue->setText(m_machineInfo.sys_vendor().data());
}

void ToolBarInformationWidget::loadCPUInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    try {
        m_cpuInfo = m_dataProvider->getDataMessage<legion::messages::CPUInfo>(
            LenovoLegionDaemon::SysFsDataProviderCPUInfo::dataType);
    } catch(DataProvider::exception_T& ex)
    {
        LOG_W(QString("Failed to load CPU information: ").append(ex.what()));
        throw;
    }
}

void ToolBarInformationWidget::displayCPUInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    // Display CPU Model
    ui->label_CPUModelValue->setText(m_cpuInfo.cpu_model().data());
    
    // Display CPU Generation
    ui->label_CPUGenerationValue->setText(m_cpuInfo.cpu_generation().data());
    
    // Display CPU Architecture
    ui->label_CPUArchitectureValue->setText(m_cpuInfo.architecture().data());
    
    // Display CPU Cache information
    QString cacheInfo;
    for (int i = 0; i < m_cpuInfo.caches_size(); i++)
    {
        const auto& cache = m_cpuInfo.caches(i);
        if (i > 0) cacheInfo += ", ";
        cacheInfo += QString("L%1 %2: %3")
            .arg(cache.level())
            .arg(cache.type().data())
            .arg(cache.size().data());
    }
    ui->label_CPUCacheValue->setText(cacheInfo);
}

void ToolBarInformationWidget::loadGPUInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    try {
        m_gpuInfo = m_dataProvider->getDataMessage<legion::messages::NvidiaNvml>(
            LenovoLegionDaemon::DataProviderNvidiaNvml::dataType);
    } catch(DataProvider::exception_T& ex)
    {
        LOG_W(QString("Failed to load GPU information: ").append(ex.what()));
        throw;
    }
}

void ToolBarInformationWidget::displayGPUInformation()
{
    LOG_T(__PRETTY_FUNCTION__);
    
    // Display GPU Name
    ui->label_GPUNameValue->setText(m_gpuInfo.name().data());
    
    // Display GPU Power Limits
    if (m_gpuInfo.has_hardware_monitor() && m_gpuInfo.hardware_monitor().has_power())
    {
        const auto& power = m_gpuInfo.hardware_monitor().power();
        QString powerInfo = QString("%1 W (Min: %2 W, Max: %3 W)")
            .arg(power.default_value() / 1000)
            .arg(power.min_value() / 1000)
            .arg(power.max_value() / 1000);
        ui->label_GPUPowerLimitValue->setText(powerInfo);
    }
}

void ToolBarInformationWidget::dataProviderEvent(const legion::messages::Notification &event)
{
    switch (event.action()) {
        case legion::messages::Notification::LENOVO_DRIVER_ADDED:
        {
            Utils::Task::executeTasks(m_defaultActionsMap["refresh"]);
        }
        break;
        case legion::messages::Notification::LENOVO_DRIVER_REMOVED:
        {
            Utils::Task::executeTasks(m_defaultActionsMap["remove"]);
        }
        break;
        default:
        break;
    }
}

void ToolBarInformationWidget::cleanup()
{
    ToolBarWidget::cleanup();
}

ToolBarInformationWidget::~ToolBarInformationWidget()
{
    delete ui;
}

void ToolBarInformationWidget::widgetEvent(const WidgetMessage &)
{}


}
