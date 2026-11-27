// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "SysFsDriverCPUIntelTcc.h"
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

SysFsDriverCPUIntelTcc::SysFsDriverCPUIntelTcc() : SysFsDriver(DRIVER_NAME,"/sys/bus/pci/devices",{}) {}

void SysFsDriverCPUIntelTcc::init()
{
    LOG_D(__PRETTY_FUNCTION__);

    clean();

    std::error_code error;

    for(const auto& entry : std::filesystem::directory_iterator(m_path,error))
    {
        if(std::filesystem::exists(entry.path() / "tcc_offset_degree_celsius"))
        {
            m_descriptor["tcc_offset"] = entry.path() / "tcc_offset_degree_celsius";
            LOG_D(QString("CPU Intel TCC offset found: ") + (entry.path() / "tcc_offset_degree_celsius").c_str());
            break;
        }
    }

    if(m_descriptor.isEmpty())
    {
        LOG_D("CPU Intel TCC offset not found");
        return;
    }

    /*
     * TjMax: the critical temperature of the package of coretemp
     */
    for(const auto& entry : std::filesystem::directory_iterator("/sys/class/hwmon",error))
    {
        if(fileText(entry.path() / "name") == "coretemp" && std::filesystem::exists(entry.path() / "temp1_crit"))
        {
            m_descriptor["tjmax"] = entry.path() / "temp1_crit";
            break;
        }
    }
}

}
