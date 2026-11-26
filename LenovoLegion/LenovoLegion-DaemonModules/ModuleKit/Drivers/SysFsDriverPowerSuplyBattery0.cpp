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

#include <algorithm>
#include <vector>

namespace LenovoLegionDaemon::Kit {


SysFsDriverPowerSuplyBattery0::SysFsDriverPowerSuplyBattery0() : SysFsDriver(DRIVER_NAME,"/sys/class/power_supply/",{"power_supply",{}}) {}

namespace {

/*
 * Content of a file of a power supply, empty when it can not be read
 */
QString powerSupplyValue(const std::filesystem::path& path)
{
    QFile file(QString::fromStdString(path.string()));

    return file.open(QIODeviceBase::ReadOnly) ? QString::fromUtf8(file.readAll()).trimmed() : QString();
}

}

void SysFsDriverPowerSuplyBattery0::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    /*
     * Battery of the notebook: the first power supply of type Battery which is not a battery of a device (scope Device:
     * mouse, headset, ...), BAT0 on Intel, often BAT1 on AMD; the name in the order of the names (BAT0 before BAT1)
     */
    m_batteryName.clear();

    std::error_code                    error;
    std::vector<std::filesystem::path> supplies;

    for(const auto& entry : std::filesystem::directory_iterator(m_path,error))
    {
        supplies.push_back(entry.path());
    }

    std::sort(supplies.begin(),supplies.end());

    for(const auto& supply : supplies)
    {
        if(powerSupplyValue(supply / "type") == "Battery" && powerSupplyValue(supply / "scope") != "Device" && std::filesystem::exists(supply / "status"))
        {
            m_batteryName = QString::fromStdString(supply.filename().string());
            break;
        }
    }

    const std::filesystem::path battery = std::filesystem::path(m_path) / m_batteryName.toStdString();

    if(!m_batteryName.isEmpty())
    {
        LOG_D(QString("Found Power supply battery driver in path: ") + battery.c_str());

        m_descriptor["powerSuplyBattery0"] = std::filesystem::path(battery).append("status");

        /*
         * Optional values: only the files the battery has (validate checks all paths of the descriptor)
         */
        for(const char* file : {"capacity","capacity_level","energy_now","energy_full","energy_full_design","charge_now","charge_full",
                                "charge_full_design","power_now","current_now","voltage_now","voltage_min_design","cycle_count",
                                "technology","manufacturer","model_name"})
        {
            const std::filesystem::path path = std::filesystem::path(battery).append(file);

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
        for(const auto& supply : supplies)
        {
            if(powerSupplyValue(supply / "type") == "Mains" && std::filesystem::exists(supply / "online"))
            {
                m_adapterName = QString::fromStdString(supply.filename().string());
                m_descriptor["ac_online"] = supply / "online";
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

    /*
     * The battery (removed, added: found again), the first battery when none was found yet
     */
    if(event.m_sysName == m_batteryName || (m_batteryName.isEmpty() && event.m_sysName.startsWith("BAT")))
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
