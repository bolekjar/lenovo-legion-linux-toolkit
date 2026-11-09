// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

/*
 * CPU power limits of the thermal mode written to the RAPL of the kernel (powercap), formerly done by the kernel module:
 *  - intel-rapl-mmio:0 (processor thermal device, the limits the CPU uses): PL1, PL2 and the PL1 time window of the mode
 *    (default values of capability data 01, in the custom mode the current values), the limits are enabled,
 *  - intel-rapl:0 (MSR): the maximum values of the mode, so the MSR limits do not restrict the MMIO limits; only when
 *    RAPL is enabled and the MMIO limits were written (locked by the BIOS: the MSR limits are not changed either).
 * Written on the start of the module, when the kernel module is loaded, on every thermal mode change (Fn+Q, set)
 * and in the custom mode at once after the limits were set (client request, restored settings).
 */

#include <CpuPower.pb.h>

#include <QString>

#include <filesystem>
namespace LenovoLegionDaemon::Kit {

class SysFsDriverManager;

class RaplPowerLimits
{
public:

    explicit RaplPowerLimits(SysFsDriverManager& drivers);

    /*
     * Limits of the thermal mode (legion::messages::PowerProfile::Profiles), cpuPower = data of the CPUPower provider.
     * Errors are logged, nothing is thrown.
     */
    void apply(int thermalMode,const legion::messages::CPUPower& cpuPower);

    static constexpr int THERMAL_MODE_CUSTOM = 255;

private:

    struct Limits
    {
        quint32 pl1Watts   = 0;
        quint32 pl2Watts   = 0;
        quint32 pl1Seconds = 0;
    };

    static Limits mmioLimits(int thermalMode,const legion::messages::CPUPower& cpuPower);
    static Limits msrLimits(int thermalMode,const legion::messages::CPUPower& cpuPower);

    /*
     * Writes the limits (0 = not known, not written), returns false when a write failed (e.g. locked by the BIOS)
     */
    bool write(const QString& domain,const Limits& limits,
               const std::filesystem::path& pl1,const std::filesystem::path& pl2,const std::filesystem::path& pl1Time);

private:

    SysFsDriverManager& m_drivers;
};

}
