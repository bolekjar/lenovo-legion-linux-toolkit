// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "../SysFsDriver.h"

namespace LenovoLegionDaemon::Kit {

/*
 * PCIe ASPM policy of the kernel: /sys/module/pcie_aspm/parameters/policy ("[default] performance powersave ...")
 */
class SysFsDriverPcieAspm : public SysFsDriver
{
public:

    struct Aspm {

        Aspm(const SysFsDriver::DescriptorType& descriptor) :
            m_policy(descriptor.value("policy"))
        {}

        const std::filesystem::path m_policy;
    };

public:

    explicit SysFsDriverPcieAspm();
    ~SysFsDriverPcieAspm() override = default;

    void init() override;

public:

    static constexpr const char* DRIVER_NAME =  "__system_pcie_aspm__";
};

}
