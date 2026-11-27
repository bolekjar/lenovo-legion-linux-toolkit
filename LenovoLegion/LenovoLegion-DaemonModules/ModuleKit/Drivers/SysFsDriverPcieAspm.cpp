// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverPcieAspm.h"
#include "../KitLog.h"

namespace LenovoLegionDaemon::Kit {

SysFsDriverPcieAspm::SysFsDriverPcieAspm() : SysFsDriver(DRIVER_NAME,"/sys/module/pcie_aspm/parameters",{}) {}

void SysFsDriverPcieAspm::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    if(std::filesystem::exists(std::filesystem::path(m_path) / "policy"))
    {
        m_descriptor["policy"] = std::filesystem::path(m_path) / "policy";
        LOG_D("PCIe ASPM policy found");
    }
}

}
