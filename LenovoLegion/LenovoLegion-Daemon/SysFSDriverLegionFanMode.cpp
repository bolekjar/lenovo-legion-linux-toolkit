// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFSDriverLegionFanMode.h"

#include <Core/LoggerHolder.h>

#include <QFile>
#include <QTextStream>

namespace LenovoLegionDaemon {

SysFSDriverLegionFanMode::SysFSDriverLegionFanMode(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-firmware-attributes/legion-wmi-fan-mode-0/attributes/",{"wmi",{}},parrent,MODULE_NAME)
{}

void SysFSDriverLegionFanMode::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    std::filesystem::path fanCurvePath = std::filesystem::path(m_path).append("fan_curve");

    if(std::filesystem::exists(fanCurvePath))
    {
        LOG_D(QString("Found Legion Fan Mode fan curve driver in path: ") + fanCurvePath.c_str());



        m_descriptor["cpu_fan_default"] = std::filesystem::path(fanCurvePath).append("cpu_fan_default");
        m_descriptor["cpu_sensor_default"] = std::filesystem::path(fanCurvePath).append("cpu_sensor_default");
        m_descriptor["gpu_fan_default"] = std::filesystem::path(fanCurvePath).append("gpu_fan_default");
        m_descriptor["gpu_sensor_default"] = std::filesystem::path(fanCurvePath).append("gpu_sensor_default");

        m_descriptor["current_value"] = std::filesystem::path(fanCurvePath).append("current_value");
        m_descriptor["display_name"] = std::filesystem::path(fanCurvePath).append("display_name");


        /*
         * Optional values for cpusen fan mode
         */

        if(std::filesystem::exists(std::filesystem::path(fanCurvePath).append("cpusen_fan_default")) &&
           std::filesystem::exists(std::filesystem::path(fanCurvePath).append("cpusen_sensor_default")))
        {
            LOG_D(QString("Found Legion Fan Mode cpusen fan mode support in path: ") + fanCurvePath.c_str());

            m_descriptor["cpusen_fan_default"]  = std::filesystem::path(fanCurvePath).append("cpusen_fan_default");
            m_descriptor["cpusen_sensor_default"] = std::filesystem::path(fanCurvePath).append("cpusen_sensor_default");
        }

        if(std::filesystem::exists(std::filesystem::path(fanCurvePath).append("sys_fan_default")) &&
            std::filesystem::exists(std::filesystem::path(fanCurvePath).append("sys_sensor_default")))
        {
            LOG_D(QString("Found Legion Fan Mode sys fan mode support in path: ") + fanCurvePath.c_str());

            m_descriptor["sys_fan_default"] = std::filesystem::path(fanCurvePath).append("sys_fan_default");
            m_descriptor["sys_sensor_default"] = std::filesystem::path(fanCurvePath).append("sys_sensor_default");
        }
    }
}

void SysFSDriverLegionFanMode::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }


    if(event.m_driver == DRIVER_NAME)
    {
        LOG_T(QString("Handling kernel event for driver: ") + m_name);

        if(event.m_action == "bind")
        {
            init();
        }

        if(event.m_action == "unbind")
        {
            clean();
        }
    }

    for(const auto& d : DEPENDENCY)
    {
        if(event.m_driver == d)
        {
            LOG_T(QString("Handling kernel event for dependency driver: ") + d);

            if(event.m_action == "bind")
            {
                init();
            }

            if(event.m_action == "unbind")
            {
                clean();
            }
        }
    }
}

}
