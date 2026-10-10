// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUAtom.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverCPUAtom::SysFsDriverCPUAtom(QObject* parrent) : SysFsDriver(DRIVER_NAME,"/sys/devices/cpu_atom/",{},parrent) {}

void SysFsDriverCPUAtom::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        LOG_D(QString("CPU Atom topology driver found in path: ") + m_path.c_str());

        m_descriptor["cpus"] = std::filesystem::path(m_path).append("cpus");
    }
    else
    {
        LOG_T(QString("CPU Atom topology driver not found in path: ") + m_path.c_str());
    }
}


}
