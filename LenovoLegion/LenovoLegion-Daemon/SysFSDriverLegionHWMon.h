// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"
#include "SysFsDriverLegion.h"

namespace LenovoLegionDaemon {


class SysFSDriverLegionHWMon : public SysFsDriver
{
public:


    struct HWMon {

        struct Legion {

            struct Fan {
                const std::filesystem::path m_input;
                const std::filesystem::path m_min;
                const std::filesystem::path m_max;
                const std::filesystem::path m_label;
            };


            struct Temp {
                const std::filesystem::path m_input;
                const std::filesystem::path m_label;
            };

            Legion(const SysFsDriver::DescriptorsInVectorType& descriptorInVector) :

                m_fans([&descriptorInVector](){
                    std::vector<Fan> fans;

                    for(const auto& desc : descriptorInVector)
                    {
                        if(desc.contains("fan_input"))
                        {
                            fans.emplace_back(Fan{
                                .m_input    = desc.value("fan_input"),
                                .m_min = desc.value("fan_min"),
                                .m_max = desc.value("fan_max"),
                                .m_label    = desc.value("fan_label")
                            });
                        }
                    }

                    return  fans;
                }()),

                m_temps([&descriptorInVector](){
                    std::vector<Temp> temp;

                    for(const auto& desc : descriptorInVector)
                    {
                        if(desc.contains("temp_input"))
                        {
                            temp.emplace_back(Temp{
                                .m_input    = desc.value("temp_input"),
                                .m_label    = desc.value("temp_label")
                            });
                        }
                    }

                    return  temp;
                }())

            {}

            const std::vector<Fan> m_fans;
            const std::vector<Temp> m_temps;
        };


        explicit HWMon(const SysFsDriver::DescriptorsInVectorType& descriptorInVector) :
            m_legion(descriptorInVector)
        {}
        const Legion         m_legion;
    };

public:

    SysFSDriverLegionHWMon(QObject * parrent);

    ~SysFSDriverLegionHWMon() override = default;

    /*
     * Init Driver
     */
    virtual void init() override;
public:

    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "hwmon";
    static constexpr const char* MODULE_NAME =  LEGION_MODULE_NAME;

};

}
