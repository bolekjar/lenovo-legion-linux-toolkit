#include "SysFSDriverLegionIntelMSR.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

SysFSDriverLegionIntelMSR::SysFSDriverLegionIntelMSR(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-intel-msr/intel-msr-0/",{},parrent,MODULE_NAME) {}

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


}
