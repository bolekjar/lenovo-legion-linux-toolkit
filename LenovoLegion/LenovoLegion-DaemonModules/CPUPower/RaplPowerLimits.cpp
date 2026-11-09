// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "RaplPowerLimits.h"

#include <SysFsDataProvider.h>
#include <SysFsDriverManager.h>
#include <Drivers/SysFsDriverIntelPowercapRapl.h>

#include <KitLog.h>

#include <QFile>

#include <algorithm>

namespace LenovoLegionDaemon::Kit {

namespace {

using Limit      = legion::messages::CPUPower::Limit;
using Descriptor = legion::messages::CPUPower::Limit::Descriptor;

const Descriptor* descriptor(const Limit& limit,int thermalMode)
{
    const auto it = limit.mode_descriptor_map().find(thermalMode);

    return it != limit.mode_descriptor_map().end() ? &it->second : nullptr;
}

/*
 * Unbuffered write, the result of the kernel store is returned (a locked limit refuses the value).
 * The udev rule 99-rapl-readonly.rules makes the limits read-only (0440): sysfs refuses to open a file without
 * any write bit even for root, so the owner write bit is added only for the write and the permissions are restored.
 */
bool writeValue(const std::filesystem::path& path,quint64 value,QString& error)
{
    std::error_code                  ec;
    const std::filesystem::perms     original = std::filesystem::status(path,ec).permissions();
    const bool                       readOnly = !ec && (original & std::filesystem::perms::owner_write) == std::filesystem::perms::none;

    if(readOnly)
    {
        std::filesystem::permissions(path,std::filesystem::perms::owner_write,std::filesystem::perm_options::add,ec);

        if(ec)
        {
            error = QString(path.c_str()).append(": write permission not added: ").append(QString::fromStdString(ec.message()));
            return false;
        }
    }

    QFile            file(QString::fromStdString(path.string()));
    const QByteArray data    = QByteArray::number(value);
    const bool       written = file.open(QIODeviceBase::WriteOnly | QIODeviceBase::Unbuffered) && file.write(data) == data.size();

    if(!written)
    {
        error = QString(path.c_str()).append(" = ").append(data).append(": ").append(file.errorString());
    }

    file.close();

    if(readOnly)
    {
        std::filesystem::permissions(path,original,std::filesystem::perm_options::replace,ec);

        if(ec)
        {
            LOG_E(QString(path.c_str()).append(": read-only permissions not restored: ").append(QString::fromStdString(ec.message())));
        }
    }

    return written;
}

}

RaplPowerLimits::RaplPowerLimits(SysFsDriverManager &drivers) :
    m_drivers(drivers)
{}

RaplPowerLimits::Limits RaplPowerLimits::mmioLimits(int thermalMode, const legion::messages::CPUPower &cpuPower)
{
    Limits limits;

    if(thermalMode == THERMAL_MODE_CUSTOM)
    {
        limits.pl1Watts   = cpuPower.cpu_ltp_limit().current_value();
        limits.pl2Watts   = cpuPower.cpu_stp_limit().current_value();
        limits.pl1Seconds = cpuPower.cpu_pl1_tau().current_value();

        return limits;
    }

    if(const auto* ltp = descriptor(cpuPower.cpu_ltp_limit(),thermalMode))  limits.pl1Watts   = ltp->default_value();
    if(const auto* stp = descriptor(cpuPower.cpu_stp_limit(),thermalMode))  limits.pl2Watts   = stp->default_value();
    if(const auto* tau = descriptor(cpuPower.cpu_pl1_tau(),thermalMode))    limits.pl1Seconds = tau->default_value();

    return limits;
}

RaplPowerLimits::Limits RaplPowerLimits::msrLimits(int thermalMode, const legion::messages::CPUPower &cpuPower)
{
    Limits limits;

    if(const auto* ltp = descriptor(cpuPower.cpu_ltp_limit(),thermalMode))  limits.pl1Watts = ltp->max_value();
    if(const auto* stp = descriptor(cpuPower.cpu_stp_limit(),thermalMode))  limits.pl2Watts = stp->max_value();

    /*
     * The longest time window of the mode (steps of the discrete data), without steps the default value
     */
    if(const auto* tau = descriptor(cpuPower.cpu_pl1_tau(),thermalMode))
    {
        limits.pl1Seconds = tau->steps_size() > 0 ? *std::max_element(tau->steps().begin(),tau->steps().end()) : tau->default_value();
    }

    return limits;
}

bool RaplPowerLimits::write(const QString &domain, const Limits &limits, const std::filesystem::path &pl1, const std::filesystem::path &pl2, const std::filesystem::path &pl1Time)
{
    QStringList errors;
    QString     error;

    const auto writeLimit = [&](const std::filesystem::path& path,quint64 value) {
        if(value > 0 && !path.empty() && !writeValue(path,value,error))
        {
            errors.push_back(error);
        }
    };

    writeLimit(pl1Time,static_cast<quint64>(limits.pl1Seconds) * 1000000);
    writeLimit(pl1,    static_cast<quint64>(limits.pl1Watts)   * 1000000);
    writeLimit(pl2,    static_cast<quint64>(limits.pl2Watts)   * 1000000);

    if(!errors.isEmpty())
    {
        LOG_W(QString("RAPL ").append(domain).append(": power limits not written (locked by the BIOS ?): ").append(errors.join(", ")));
        return false;
    }

    LOG_D(QString("RAPL ").append(domain).append(": PL1 ").append(QString::number(limits.pl1Watts)).append(" W, PL2 ").append(QString::number(limits.pl2Watts))
          .append(" W, PL1 time window ").append(QString::number(limits.pl1Seconds)).append(" s"));

    return true;
}

void RaplPowerLimits::apply(int thermalMode, const legion::messages::CPUPower &cpuPower)
{
    try {
        if(!m_drivers.isLoaded(SysFsDriverIntelPowercapRapl::DRIVER_NAME))
        {
            LOG_D("RAPL: powercap is not available, power limits of the thermal mode are not written");
            return;
        }

        const auto&                                         descriptor = m_drivers.getDriverDesriptor(SysFsDriverIntelPowercapRapl::DRIVER_NAME);
        const SysFsDriverIntelPowercapRapl::IntelPowercapRaplMMIO mmio(descriptor);
        const SysFsDriverIntelPowercapRapl::IntelPowercapRapl     msr(descriptor);

        LOG_D(QString("RAPL: power limits of the thermal mode ").append(QString::number(thermalMode)));

        if(mmio.m_ltp_power_limit_uw.empty())
        {
            LOG_D("RAPL: intel-rapl-mmio is not available");
            return;
        }

        /*
         * The limits of the mode are used only when enabled (the kernel module enabled PL1 and PL2 as well)
         */
        if(!mmio.m_enabled.empty() && SysFsDataProvider::getData(mmio.m_enabled).trimmed() == "0")
        {
            QString error;

            if(!writeValue(mmio.m_enabled,1,error))
            {
                LOG_W(QString("RAPL intel-rapl-mmio:0: not enabled: ").append(error));
            }
        }

        if(!write("intel-rapl-mmio:0",mmioLimits(thermalMode,cpuPower),mmio.m_ltp_power_limit_uw,mmio.m_stp_power_limit_uw,mmio.m_ltp_time_window_us))
        {
            return;
        }

        if(msr.m_ltp_power_limit_uw.empty() || msr.m_enabled.empty() || SysFsDataProvider::getData(msr.m_enabled).trimmed() != "1")
        {
            LOG_D("RAPL intel-rapl:0: not available or disabled, only the MMIO limits are written");
            return;
        }

        write("intel-rapl:0",msrLimits(thermalMode,cpuPower),msr.m_ltp_power_limit_uw,msr.m_stp_power_limit_uw,msr.m_ltp_time_window_us);
    }
    catch(const std::exception& ex)
    {
        LOG_W(QString("RAPL: power limits of the thermal mode not written: ").append(ex.what()));
    }
}

}
