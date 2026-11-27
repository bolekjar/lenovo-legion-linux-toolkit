// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverGPURuntimePM.h"
#include "../KitLog.h"

#include <QFile>

namespace LenovoLegionDaemon::Kit {

namespace {

QString fileText(const std::filesystem::path& path)
{
    QFile file(QString::fromStdString(path.string()));

    return file.open(QIODeviceBase::ReadOnly) ? QString::fromUtf8(file.readAll()).trimmed() : QString();
}

}

SysFsDriverGPURuntimePM::SysFsDriverGPURuntimePM() : SysFsDriver(DRIVER_NAME,"/sys/bus/pci/devices",{}) {}

void SysFsDriverGPURuntimePM::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    std::error_code error;

    for(const auto& entry : std::filesystem::directory_iterator(m_path,error))
    {
        const std::filesystem::path device = entry.path();

        /*
         * Display controller (class 0x03xxxx) of NVIDIA with the runtime power management of the PCI core
         */
        if(fileText(device / "vendor") != "0x10de" || !fileText(device / "class").startsWith("0x03") ||
           !std::filesystem::exists(device / "power" / "control"))
        {
            continue;
        }

        m_descriptor["device"]         = device;
        m_descriptor["control"]        = device / "power" / "control";
        m_descriptor["runtime_status"] = device / "power" / "runtime_status";

        LOG_D(QString("Discrete NVIDIA GPU found: ") + device.c_str());
        break;
    }
}

}
