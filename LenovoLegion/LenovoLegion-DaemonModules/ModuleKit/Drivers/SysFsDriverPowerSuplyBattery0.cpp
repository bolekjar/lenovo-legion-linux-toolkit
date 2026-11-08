// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverPowerSuplyBattery0.h"

#include "../KitLog.h"

#include <QFile>

namespace LenovoLegionDaemon::Kit {


SysFsDriverPowerSuplyBattery0::SysFsDriverPowerSuplyBattery0() : SysFsDriver(DRIVER_NAME,"/sys/class/power_supply/BAT0/",{"power_supply",{}}) {}

void SysFsDriverPowerSuplyBattery0::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path)))
    {
        LOG_D(QString("Found Power supply battery driver in path: ") + m_path.c_str());

        m_descriptor["powerSuplyBattery0"] = std::filesystem::path(m_path).append("status");

        /*
         * Optional values: only the files the battery has (validate checks all paths of the descriptor)
         */
        for(const char* file : {"capacity","capacity_level","energy_now","energy_full","energy_full_design","charge_now","charge_full",
                                "charge_full_design","power_now","current_now","voltage_now","voltage_min_design","cycle_count",
                                "technology","manufacturer","model_name"})
        {
            const std::filesystem::path path = std::filesystem::path(m_path).append(file);

            if(std::filesystem::exists(path))
            {
                m_descriptor[file] = path;
            }
        }
    }
    else
    {
        LOG_T(QString("Power supply battery driver not found in path: ") + m_path.c_str());
    }

    /*
     * AC adapter: the first power supply of type Mains (ADP0, AC, ACAD, ...)
     */
    m_adapterName.clear();

    if(!m_descriptor.isEmpty())
    {
        std::error_code error;

        for(const auto& entry : std::filesystem::directory_iterator("/sys/class/power_supply",error))
        {
            QFile type(QString::fromStdString(entry.path() / "type"));

            if(type.open(QIODeviceBase::ReadOnly) && QString::fromUtf8(type.readAll()).trimmed() == "Mains" && std::filesystem::exists(entry.path() / "online"))
            {
                m_adapterName = QString::fromStdString(entry.path().filename().string());
                m_descriptor["ac_online"] = entry.path() / "online";
                LOG_D(QString("Found AC adapter: ") + m_adapterName);
                break;
            }
        }
    }
}

void SysFsDriverPowerSuplyBattery0::handleKernelEvent(const KernelEvent::Event &event)
{
    LOG_D(__PRETTY_FUNCTION__ + QString(": Kernel event received ACTION=") + event.m_action + ", DRIVER=" + event.m_driver + ", SYSNAME=" + event.m_sysName + ", SUBSYSTEM=" + event.m_subSystem + ", DEVPATH=" + event.m_devPath);

    if(m_blockKernelEvent)
    {
        LOG_T(QString("Kernel event blocked for driver: ") + m_name);
        return;
    }

    /*
     * AC adapter plugged / unplugged: the same notification as a change of the battery
     */
    if(!m_adapterName.isEmpty() && event.m_sysName == m_adapterName && event.m_action == "change")
    {
        emit kernelEvent({
            .m_driverName = DRIVER_NAME,
            .m_action = SubsystemEvent::Action::CHANGED,
            .m_DriverSpecificEventType = "changed",
            .m_DriverSpecificEventValue = {}
        });
        return;
    }

    if(event.m_sysName == "BAT0")
    {
        if(event.m_action == "change")
        {
            emit kernelEvent({
                .m_driverName = DRIVER_NAME,
                .m_action = SubsystemEvent::Action::CHANGED,
                .m_DriverSpecificEventType = "changed",
                .m_DriverSpecificEventValue = {}
            });
        }
        else
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


}
