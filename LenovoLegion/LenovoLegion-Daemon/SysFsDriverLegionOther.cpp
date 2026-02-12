// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFsDriverLegionOther.h"

#include <Core/LoggerHolder.h>

#include <QFile>
#include <QTextStream>
#include <QString>

namespace LenovoLegionDaemon {

SysFsDriverLegionOther::SysFsDriverLegionOther(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/legion-firmware-attributes/legion-wmi-other-0/attributes/",{"wmi",{}},parrent,MODULE_NAME)
{}

void SysFsDriverLegionOther::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    auto buildSstandardAttributesCapData01 = [this] (std::filesystem::path path, const QString& prefix)
    {
        m_descriptor[prefix + "current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor[prefix + "default_value"] = std::filesystem::path(path).append("default_value");
        m_descriptor[prefix + "display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor[prefix + "max_value"] = std::filesystem::path(path).append("max_value");
        m_descriptor[prefix + "min_value"] = std::filesystem::path(path).append("min_value");
        m_descriptor[prefix + "scalar_increment"] = std::filesystem::path(path).append("scalar_increment");
        m_descriptor[prefix + "steps"] = std::filesystem::path(path).append("steps");
        m_descriptor[prefix + "supported"] = std::filesystem::path(path).append("supported");
        m_descriptor[prefix + "type"] = std::filesystem::path(path).append("type");
    };

    auto buildSstandardAttributesCapData00 = [this] (std::filesystem::path path, const QString& prefix)
    {
        m_descriptor[prefix + "current_value"] = std::filesystem::path(path).append("current_value");
        m_descriptor[prefix + "default_value"] = std::filesystem::path(path).append("default_value");
        m_descriptor[prefix + "display_name"] = std::filesystem::path(path).append("display_name");
        m_descriptor[prefix + "supported"] = std::filesystem::path(path).append("supported");
        m_descriptor[prefix + "type"] = std::filesystem::path(path).append("type");
    };


    std::vector<QString> standardSttributesCapData01 = {
                                                        "apus_pptp_limit","cpu_clp_limit","cpu_ltp_limit","cpu_pl1_tau","cpu_pp_limit","cpu_stp_limit","cpu_tmp_limit",
                                                        "gpu_configurable_tgp","gpu_power_boost","gpu_temperature_limit","gpu_to_cpu_dynamic_boost","gpu_total_onac"
                                                       };

    for (const auto& attr : standardSttributesCapData01)
    {
        std::filesystem::path path = std::filesystem::path(m_path).append(attr.toStdString().c_str());
        if(std::filesystem::exists(path))
        {
            LOG_D(QString("Found Legion Other ").append(attr).append(" driver in path: ") + path.c_str());

            buildSstandardAttributesCapData01(path,attr.toStdString().c_str());
        }
    };


    std::vector<QString> standardSttributesCapData00 = {"fan_full_speed","god_mode_fnq_switchable"};
    for (const auto& attr : standardSttributesCapData00)
    {
        std::filesystem::path path = std::filesystem::path(m_path).append(attr.toStdString().c_str());
        if(std::filesystem::exists(path))
        {
            LOG_D(QString("Found Legion Other ").append(attr).append(" driver in path: ") + path.c_str());

            buildSstandardAttributesCapData00(path,attr.toStdString().c_str());
        }
    };
}

void SysFsDriverLegionOther::handleKernelEvent(const KernelEvent::Event &event)
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
