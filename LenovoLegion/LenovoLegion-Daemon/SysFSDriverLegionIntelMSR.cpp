#include "SysFSDriverLegionIntelMSR.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

SysFSDriverLegionIntelMSR::SysFSDriverLegionIntelMSR(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-intel-msr/intel-msr-0/",{"legion-intel-msr",{}},parrent,MODULE_NAME) {}

void SysFSDriverLegionIntelMSR::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(m_path))
    {
        LOG_D(QString("Legion Intel MSR driver found in path: ") + m_path.c_str());

        m_descriptor["analogio_max_overvolt"] = std::filesystem::path(m_path).append("analogio_max_overvolt");
        m_descriptor["analogio_max_undervolt"] = std::filesystem::path(m_path).append("analogio_max_undervolt");
        m_descriptor["analogio_offset"] = std::filesystem::path(m_path).append("analogio_offset");
        m_descriptor["analogio_offset_ctrl_supported"] = std::filesystem::path(m_path).append("analogio_offset_ctrl_supported");

        m_descriptor["cache_max_overvolt"] = std::filesystem::path(m_path).append("cache_max_overvolt");
        m_descriptor["cache_max_undervolt"] = std::filesystem::path(m_path).append("cache_max_undervolt");
        m_descriptor["cache_offset"] = std::filesystem::path(m_path).append("cache_offset");
        m_descriptor["cache_offset_ctrl_supported"] = std::filesystem::path(m_path).append("cache_offset_ctrl_supported");

        m_descriptor["cpu_max_overvolt"] = std::filesystem::path(m_path).append("cpu_max_overvolt");
        m_descriptor["cpu_max_undervolt"] = std::filesystem::path(m_path).append("cpu_max_undervolt");
        m_descriptor["cpu_offset"] = std::filesystem::path(m_path).append("cpu_offset");
        m_descriptor["cpu_offset_ctrl_supported"] = std::filesystem::path(m_path).append("cpu_offset_ctrl_supported");

        m_descriptor["gpu_max_overvolt"] = std::filesystem::path(m_path).append("gpu_max_overvolt");
        m_descriptor["gpu_max_undervolt"] = std::filesystem::path(m_path).append("gpu_max_undervolt");
        m_descriptor["gpu_offset"] = std::filesystem::path(m_path).append("gpu_offset");
        m_descriptor["gpu_offset_ctrl_supported"] = std::filesystem::path(m_path).append("gpu_offset_ctrl_supported");

        m_descriptor["uncore_max_overvolt"] = std::filesystem::path(m_path).append("uncore_max_overvolt");
        m_descriptor["uncore_max_undervolt"] = std::filesystem::path(m_path).append("uncore_max_undervolt");
        m_descriptor["uncore_offset"] = std::filesystem::path(m_path).append("uncore_offset");
        m_descriptor["uncore_offset_ctrl_supported"] = std::filesystem::path(m_path).append("uncore_offset_ctrl_supported");
    }
}

void SysFSDriverLegionIntelMSR::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(event.m_driver == DRIVER_NAME)
    {
         LOG_D(QString("Legion Intel MSR driver received kernel event: ") + event.m_action + ", " + event.m_sysName);

        if(event.m_action == "change")
        {
            emit kernelEvent({
                .m_driverName = DRIVER_NAME,
                .m_action = SubsystemEvent::Action::CHANGED,
                .m_DriverSpecificEventType = event.m_properties.value("EVENT_TYPE", ""),
                .m_DriverSpecificEventValue = event.m_properties.value("EVENT_VALUE", "")
            });
        }
    }
}


}
