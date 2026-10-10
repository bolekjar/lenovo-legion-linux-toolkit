// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDriverCPUInfo.h"
#include "Core/LoggerHolder.h"


namespace LenovoLegionDaemon {

SysFsDriverCPUInfo::SysFsDriverCPUInfo(QObject *parent): SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/",{},parent)
{}

void SysFsDriverCPUInfo::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    const std::filesystem::path cpu0_path = std::filesystem::path(m_path).append("cpu0");
    
    if (std::filesystem::exists(std::filesystem::path(m_path).append("modalias"))){
        m_descriptor["modalias"] = std::filesystem::path(m_path).append("modalias");
    }
    
    if (std::filesystem::exists(std::filesystem::path(cpu0_path).append("topology/physical_package_id"))) {
        m_descriptor["package_id"] = std::filesystem::path(cpu0_path).append("topology/physical_package_id");
    }
    
    const std::filesystem::path cache_path = std::filesystem::path(cpu0_path).append("cache");
    if (std::filesystem::exists(cache_path)) {

        LOG_D(QString("Found CPU Info cache driver in path: ") + cache_path.c_str());

        for (int i = 0; i < 10; i++) {
            std::string index_path = std::filesystem::path(cache_path).append("index").string() + std::to_string(i);

            if (std::filesystem::exists(index_path) && std::filesystem::is_directory(index_path)) {
                DescriptorType cache_descriptor;

                if (std::filesystem::exists(std::filesystem::path(index_path).append("level"))) {
                    cache_descriptor["cache_level"] = std::filesystem::path(index_path).append("level");
                }

                if (std::filesystem::exists(std::filesystem::path(index_path).append("type"))) {
                    cache_descriptor["cache_type"] = std::filesystem::path(index_path).append("type");
                }

                if (std::filesystem::exists(std::filesystem::path(index_path).append("size"))) {
                    cache_descriptor["cache_size"] = std::filesystem::path(index_path).append("size");
                }

                if (!cache_descriptor.empty()) {
                    m_descriptorsInVector.push_back(cache_descriptor);
                }
            }
        }
    }
}

}
