// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUAmdPState.h"
#include "../KitLog.h"

#include <QFile>

#include <algorithm>

namespace LenovoLegionDaemon::Kit {

namespace {

const std::filesystem::path CPUFREQ_PATH = "/sys/devices/system/cpu/cpufreq";

/*
 * Scaling driver of a cpufreq policy ("amd-pstate", "amd-pstate-epp")
 */
QString scalingDriver(const std::filesystem::path& policy)
{
    QFile file(QString::fromStdString((policy / "scaling_driver").string()));

    return file.open(QIODeviceBase::ReadOnly) ? QString::fromUtf8(file.readAll()).trimmed() : QString();
}

}

SysFsDriverCPUAmdPState::SysFsDriverCPUAmdPState() : SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/amd_pstate",{}) {}

void SysFsDriverCPUAmdPState::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    const std::filesystem::path status = std::filesystem::path(m_path) / "status";

    if(!std::filesystem::exists(status))
    {
        LOG_D(QString("CPU AMD P-State driver not found in path: ") + m_path.c_str());
        return;
    }

    LOG_D(QString("CPU AMD P-State driver found in path: ") + m_path.c_str());

    m_descriptor["status"] = status;

    if(std::filesystem::exists(std::filesystem::path(m_path) / "prefcore"))
    {
        m_descriptor["prefcore"] = std::filesystem::path(m_path) / "prefcore";
    }

    /*
     * Policies handled by amd-pstate (none in the mode "disable"): boost and EPP
     */
    std::error_code                    error;
    std::vector<std::filesystem::path> policies;

    for(const auto& entry : std::filesystem::directory_iterator(CPUFREQ_PATH,error))
    {
        if(entry.path().filename().string().starts_with("policy") && scalingDriver(entry.path()).startsWith("amd-pstate"))
        {
            policies.push_back(entry.path());
        }
    }

    std::sort(policies.begin(),policies.end());

    if(std::filesystem::exists(CPUFREQ_PATH / "boost"))
    {
        m_descriptor["boost"] = CPUFREQ_PATH / "boost";
    }
    else
    {
        for(const auto& policy : policies)
        {
            if(std::filesystem::exists(policy / "boost"))
            {
                m_descriptor[QString("boost/").append(policy.filename().c_str())] = policy / "boost";
            }
        }
    }

    for(const auto& policy : policies)
    {
        if(std::filesystem::exists(policy / "energy_performance_preference"))
        {
            m_descriptor[QString("epp/").append(policy.filename().c_str())] = policy / "energy_performance_preference";

            if(!m_descriptor.contains("epp_available") && std::filesystem::exists(policy / "energy_performance_available_preferences"))
            {
                m_descriptor["epp_available"] = policy / "energy_performance_available_preferences";
            }
        }
    }
}

}
