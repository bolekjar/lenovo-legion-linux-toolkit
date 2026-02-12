// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "SysFSDriverLegionHWMon.h"

#include <Core/LoggerHolder.h>

#include <QFile>
#include <QTextStream>


namespace LenovoLegionDaemon {


SysFSDriverLegionHWMon::SysFSDriverLegionHWMon(QObject *parrent) : SysFsDriver(DRIVER_NAME,"/sys/class/hwmon/",{},parrent,MODULE_NAME)
{}

void SysFSDriverLegionHWMon::init()
{
    LOG_T(__PRETTY_FUNCTION__);

    clean();

    for(const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(m_path)))
    {

        std::filesystem::path  pathToName = std::filesystem::path(entry.path()).append(std::string("name"));

        if(std::filesystem::exists(pathToName))
        {
            QFile file(pathToName);

            if(!file.open(QIODeviceBase::ReadOnly))
            {
                continue;
            }

            QString driverName = QTextStream(&file).readAll();


            if(driverName.trimmed() == "legion")
            {
                LOG_D(QString("Found Legion HWMon driver in path: ") + entry.path().c_str());
                /*
                 * Fans
                 */
                for(const auto& entry : std::filesystem::directory_iterator(std::filesystem::path(entry)))
                {
                    if(entry.is_regular_file() && entry.path().filename().begin()->string().find("fan") != std::string::npos)
                    {
                        qsizetype fanCount = 0;
                        char name[64] = {0};

                        sscanf(entry.path().filename().string().c_str(),"fan%llu_%s",&fanCount,name);

                        m_descriptorsInVector.resize(std::max(fanCount,m_descriptorsInVector.size()));
                        m_descriptorsInVector[fanCount - 1]["fan_" + QString(name)] = entry.path();

                        LOG_T(QString("Legion HWMon Fan(") + entry.path().filename().string().c_str() + ")driver file: " + entry.path().c_str());
                    }

                    if(entry.is_regular_file() && entry.path().filename().string().find("temp") != std::string::npos)
                    {
                        qsizetype tempCount = 0;
                        char name[64] = {0};

                        sscanf(entry.path().filename().string().c_str(),"temp%llu_%s",&tempCount,name);

                        m_descriptorsInVector.resize(std::max(tempCount,m_descriptorsInVector.size()));
                        m_descriptorsInVector[tempCount - 1]["temp_" + QString(name)] = entry.path();

                        LOG_T(QString("Legion HWMon Temp(") + entry.path().filename().string().c_str() + ")driver file: " + entry.path().c_str());
                    }
                }
            }
        }

    }
}
}
