// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUIntelUncore.h"
#include "../KitLog.h"

namespace LenovoLegionDaemon::Kit {

SysFsDriverCPUIntelUncore::SysFsDriverCPUIntelUncore() : SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/intel_uncore_frequency",{}) {}

void SysFsDriverCPUIntelUncore::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    std::error_code error;

    /*
     * Domains: package_XX_die_YY (intel_uncore_frequency) or uncoreNN (TPMI), the ones with the limits of the hardware
     */
    for(const auto& entry : std::filesystem::directory_iterator(m_path,error))
    {
        const std::filesystem::path dir  = entry.path();
        const QString               name = QString::fromStdString(dir.filename().string());

        if(!entry.is_directory() || !std::filesystem::exists(dir / "min_freq_khz") || !std::filesystem::exists(dir / "initial_max_freq_khz"))
        {
            continue;
        }

        for(const char* file : {"min_freq_khz","max_freq_khz","initial_min_freq_khz","initial_max_freq_khz","current_freq_khz"})
        {
            if(std::filesystem::exists(dir / file))
            {
                m_descriptor[name + "/" + file] = dir / file;
            }
        }

        LOG_D(QString("CPU Intel uncore frequency domain found: ") + dir.c_str());
    }

    if(m_descriptor.isEmpty())
    {
        LOG_D(QString("CPU Intel uncore frequency driver not found in path: ") + m_path.c_str());
    }
}

}
