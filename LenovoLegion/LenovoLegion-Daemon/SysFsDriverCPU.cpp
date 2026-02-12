// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPU.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {


SysFsDriverCPU::SysFsDriverCPU(QObject * parrent) : SysFsDriver(DRIVER_NAME,"/sys/devices/system/cpu/",{},parrent) {}

void SysFsDriverCPU::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(m_path))
    {

        LOG_D(QString("CPU driver found in path: ") + m_path.c_str());

        /*
         * CPU Topology
         */
        m_descriptor["kernelMax"] = std::filesystem::path(m_path).append("kernel_max");
        m_descriptor["offline"] = std::filesystem::path(m_path).append("offline");
        m_descriptor["online"] = std::filesystem::path(m_path).append("online");
        m_descriptor["possible"] = std::filesystem::path(m_path).append("possible");
        m_descriptor["present"] = std::filesystem::path(m_path).append("present");

        /*
         * CPU SMT
         */
        if(std::filesystem::exists(std::filesystem::path(m_path).append("smt")))
        {
            LOG_D(QString("CPU SMT driver found in path: ") + std::filesystem::path(m_path).append("smt").c_str());

            m_descriptor["smtActive"] = std::filesystem::path(m_path).append("smt").append("active");
            m_descriptor["smtControl"] = std::filesystem::path(m_path).append("smt").append("control");
        }
        else
        {
            LOG_T(QString("CPU SMT driver not found in path: ") + std::filesystem::path(m_path).append("smt").c_str());
        }
    }
    else
    {
        LOG_T(QString("CPU driver not found in path: ") + m_path.c_str());
    }
}

}
