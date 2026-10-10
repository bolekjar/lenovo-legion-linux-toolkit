// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUXList.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

SysFsDriverCPUXList::SysFsDriverCPUXList(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/",{"cpu",{}},parrent) {}

void SysFsDriverCPUXList::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    /*
     * CPUX driver
     */
    for(const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(m_path)))
    {
        if(entry.is_directory())
        {

            auto dir = (--entry.path().end())->string();
            if(dir.find("cpu") != std::string::npos && dir.substr(std::string("cpu").size()).size() > 0 && std::isdigit(dir.substr(std::string("cpu").size())[0]))
            {
                LOG_D(QString("Found CPUX driver in path: ") + entry.path().c_str());

                qsizetype cpuIndex = std::stoi(dir.substr(std::string("cpu").size()));

                m_descriptorsInVector.resize(std::max(cpuIndex + 1,m_descriptorsInVector.size()));

                if(std::filesystem::exists(std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("cpufreq")))
                {
                    LOG_D(QString("Found CPUX cpufreq driver in path: ") + std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("cpufreq").c_str());

                    m_descriptorsInVector[cpuIndex]["affectedCpus"]                    = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("affected_cpus");
                    if(std::filesystem::exists(std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("base_frequency")))
                    {
                        m_descriptorsInVector[cpuIndex]["cpuBaseFreq"]                     = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("base_frequency");
                    }
                    m_descriptorsInVector[cpuIndex]["cpuInfoMinFreq"]                  = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("cpuinfo_min_freq");
                    m_descriptorsInVector[cpuIndex]["cpuInfoMaxFreq"]                  = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("cpuinfo_max_freq");
                    m_descriptorsInVector[cpuIndex]["cpuScalingAvailableGovernors"]    = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("scaling_available_governors");
                    m_descriptorsInVector[cpuIndex]["cpuScalingGovernor"]              = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("scaling_governor");
                    m_descriptorsInVector[cpuIndex]["cpuScalingCurFreq"]               = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("scaling_cur_freq");
                    m_descriptorsInVector[cpuIndex]["cpuScalingMinFreq"]               = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("scaling_min_freq");
                    m_descriptorsInVector[cpuIndex]["cpuScalingMaxFreq"]               = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string( cpuIndex)).append("cpufreq").append("scaling_max_freq");


                    if(std::filesystem::exists(std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology")))
                    {
                        LOG_D(QString("Found CPUX topology driver in path: ") + std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").c_str());

                        m_descriptorsInVector[cpuIndex]["clusterId"]                       = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("cluster_id");
                        m_descriptorsInVector[cpuIndex]["physicalPackageId"]               = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("physical_package_id");
                        m_descriptorsInVector[cpuIndex]["coreId"]                          = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("core_id");
                        m_descriptorsInVector[cpuIndex]["dieId"]                           = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("die_id");
                        m_descriptorsInVector[cpuIndex]["clusterCpusList"]                 = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("cluster_cpus_list");
                        m_descriptorsInVector[cpuIndex]["packageCpusList"]                 = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("package_cpus_list");
                        m_descriptorsInVector[cpuIndex]["dieCpusList"]                     = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("die_cpus_list");
                        m_descriptorsInVector[cpuIndex]["coreCpusList"]                    = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("core_cpus_list");
                        m_descriptorsInVector[cpuIndex]["coreSiblingsList"]                = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("core_siblings_list");
                        m_descriptorsInVector[cpuIndex]["threadSiblingsList"]              = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("topology").append("thread_siblings_list");
                    }

                    if(std::filesystem::exists(std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("online")))
                    {
                        m_descriptorsInVector[cpuIndex]["cpuOnline"]                       = std::filesystem::path(m_path).append(std::string("cpu") + std::to_string(cpuIndex)).append("online");
                    }
                }
            }
        }
    }
}

void SysFsDriverCPUXList::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    if(event.m_driver == DRIVER_NAME)
    {
        init();
        validate();

        emit kernelEvent({
            .m_driverName = DRIVER_NAME,
            .m_action = SubsystemEvent::Action::RELOADED,
            .m_DriverSpecificEventType = "reloaded",
            .m_DriverSpecificEventValue = {}
        });
    }
}

}

