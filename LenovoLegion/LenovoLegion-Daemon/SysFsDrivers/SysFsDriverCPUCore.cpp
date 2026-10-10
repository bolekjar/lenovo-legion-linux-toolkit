// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUCore.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverCPUCore::SysFsDriverCPUCore(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/devices/cpu_core/",{},parrent) {}

void SysFsDriverCPUCore::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        LOG_D(QString("CPU Core topology driver found in path: ") + m_path.c_str());

        m_descriptor["cpus"] = std::filesystem::path(m_path).append("cpus");
    }
    else
    {
        LOG_T(QString("CPU Core topology driver not found in path: ") + m_path.c_str());
    }
}


}
