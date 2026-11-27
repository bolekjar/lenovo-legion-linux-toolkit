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
 * Runtime power management of the discrete NVIDIA GPU: the PCI display device of NVIDIA (vendor 0x10de, class 0x03),
 * its power/control (auto / on) and power/runtime_status
 */
class SysFsDriverGPURuntimePM : public SysFsDriver
{
public:

    struct RuntimePM {

        RuntimePM(const SysFsDriver::DescriptorType& descriptor) :
            m_control(descriptor.value("control")),
            m_status(descriptor.value("runtime_status")),
            m_device(descriptor.value("device"))
        {}

        const std::filesystem::path m_control;   // power/control (rw)
        const std::filesystem::path m_status;    // power/runtime_status
        const std::filesystem::path m_device;    // the PCI device (its name is the address)
    };

public:

    explicit SysFsDriverGPURuntimePM();
    ~SysFsDriverGPURuntimePM() override = default;

    void init() override;

public:

    static constexpr const char* DRIVER_NAME =  "__system_gpu_runtime_pm__";
};

}
