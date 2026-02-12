// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFSDriverLegionGameZone.h"

#include <Core/LoggerHolder.h>

#include <QFile>
#include <QTextStream>



namespace LenovoLegionDaemon {

SysFSDriverLegionGameZone::SysFSDriverLegionGameZone(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-firmware-attributes/legion-wmi-gamezone-0/attributes/",{"wmi",{}},parrent,MODULE_NAME)
{}


void SysFSDriverLegionGameZone::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    std::filesystem::path path = std::filesystem::path(m_path).append("smart_fan");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone smart fan driver in path: ") + path.c_str());

        m_descriptor["smart_fan_current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor["smart_fan_display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor["smart_fan_supported"] = std::filesystem::path(path).append("supported");
        m_descriptor["smart_fan_extreme_supported"] = std::filesystem::path(path).append("extreme_supported");
    }

    path = std::filesystem::path(m_path).append("disable_tp");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone disable tp driver in path: ") + path.c_str());

        m_descriptor["disable_tp_current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor["disable_tp_display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor["disable_tp_supported"] = std::filesystem::path(path).append("supported");
    }

    path = std::filesystem::path(m_path).append("disable_win_key");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone disable win key driver in path: ") + path.c_str());

        m_descriptor["disable_win_key_current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor["disable_win_key_display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor["disable_win_key_supported"] = std::filesystem::path(path).append("supported");
    }

    path = std::filesystem::path(m_path).append("gsync");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone gsync driver in path: ") + path.c_str());

        m_descriptor["gsync_current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor["gsync_display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor["gsync_supported"] = std::filesystem::path(path).append("supported");
    }

    path = std::filesystem::path(m_path).append("igpu_mode");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone igpu mode driver in path: ") + path.c_str());

        m_descriptor["igpu_mode_current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor["igpu_mode_display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor["igpu_mode_supported"] = std::filesystem::path(path).append("supported");
    }

    path = std::filesystem::path(m_path).append("other");
    if(std::filesystem::exists(path))
    {
        LOG_D(QString("Found Legion Game Zone other driver in path: ") + path.c_str());

        m_descriptor["other_get_power_charge_mode"] = std::filesystem::path(path).append("get_power_charge_mode");
        m_descriptor["other_get_thermal_mode"] = std::filesystem::path(path).append("get_thermal_mode");
    }
}

void SysFSDriverLegionGameZone::handleKernelEvent(const KernelEvent::Event &event)
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
}

}
