// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include "SysFsDriver.h"



namespace LenovoLegionDaemon {


class SysFsDriverACPIPlatformProfile : public SysFsDriver
{

public:

    struct ACPIPlatformProfile {

        ACPIPlatformProfile(const SysFsDriver::DescriptorType& descriptor) :
            m_acpiPlatformProfile(descriptor["acpiPlatformProfile"])
        {}

        const std::filesystem::path m_acpiPlatformProfile;
    };

public:

    SysFsDriverACPIPlatformProfile(QObject * parrent);


    ~SysFsDriverACPIPlatformProfile() override = default;


    /*
     * Init Driver
     */
    virtual void init() override;


    virtual void handleKernelEvent(const KernelEvent::Event& event) override;


    /*
     * Driver name, system driver __ prefix is used to mark  system driver no modprobe loadable
     */
    static constexpr const char* DRIVER_NAME =  "platform_profile";
};

}
