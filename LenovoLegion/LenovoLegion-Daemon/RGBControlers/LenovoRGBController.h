// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */


#pragma once

#include "LenovoUSBController.h"

#include "RGBController.h"

#include <QtTypes>

#include <memory>

namespace LenovoLegionDaemon {

class LenovoRGBController : public RGBController
{

protected:

    enum SupportedModes : quint8
    {
        MODE_SCREW_RAINBOW                  = 0x01,
        MODE_RAINBOW_WAVE                   = 0x02,
        MODE_COLOR_CHANGE                   = 0x03,
        MODE_COLOR_PULSE                    = 0x04,
        MODE_COLOR_WAVE                     = 0x05,
        MODE_SMOOTH                         = 0x06,
        MODE_RAIN                           = 0x07,
        MODE_RIPPLE                         = 0x08,
        MODE_AUDIO_BOUNCE_LIGHTING          = 0x09,
        MODE_AUDIO_RIPPLE_LIGHTING          = 0x0A,
        MODE_ALWAYS                         = 0x0B,
        MODE_TYPE_LIGHTING                  = 0x0C,
        MODE_LEGION_AURASYNC                = 0x0D
    };

    enum
    {
        BRIGHTNESS_MIN                = 0x00,
        BRIGHTNESS_MAX                = 0x09,
        SPEED_MIN                     = 0x01,
        SPEED_MAX                     = 0x03
    };

    enum KeyBoardLayouts : quint8
    {
        ISO_LAYOUT                     = 0x00,
        ANSI_LAYOUT                    = 0x01
    };

public:

    DEFINE_EXCEPTION(LenovoLegionDaemon);

    enum ERROR_CODES : int
    {
        DIRECT_CONTROL_NOT_SUPPORTED = 1
    };

public:

    LenovoRGBController(LenovoUSBController* controller_ptr,
                        const Profiles&                profiles,
                        const Brightnesses&            britnesses,
                        const std::string&             name,
                        const std::string&             vendor,
                        const std::string&             description,
                        const std::string&             serial,
                        const std::string&             location,
                        device_type                    type,
                        const std::vector<mode>&       modes,
                        const std::vector<zone>&       zones,
                        unsigned int                   maxEffects,
                        bool                           hasLogo
                        );
    virtual ~LenovoRGBController();

    void        DeviceUpdateEfects()                   override;
    void        DeviceResetEffectsToDefault()          override;
    void        DeviceRefreshEffects()                 override;

    void        DeviceUpdateProfile()                  override;
    void        DeviceRefreshProfile(int expectedProfile = -1)  override;

    void        DeviceUpdateBrightness()               override;
    void        DeviceRefreshBrightness()              override;

    void        DeviceRefresh(int expectedProfile = -1) override;

    std::vector<RGBColor> DeviceGetState()     const   override;

    uint16_t                GetVendorID()         const   override;
    uint16_t                GetProductID()        const   override;

    void            DeviceUpdateLogoState()       override;
    void            DeviceRefreshLogoState()      override;

private:
    void readActiveProfileSettings();

    /*
     * Serialization from controler model
     */
    unsigned int             fromControlerColorMode(const unsigned int  modeFlags,
                                                    const uint8_t color_mode)                               const;
    unsigned int             fromControlerDirection(const uint8_t direction)                                const;
    unsigned int             fromControlerSpin(const uint8_t spin)                                          const;
    unsigned int             fromControlerProfile(uint8_t profile)                                          const;
    unsigned int             fromControlerBrightness(uint8_t brightness)                                    const;


protected:

    /*
     * Serialization to controler model
     */
    uint8_t                                             toControlerDirection(unsigned int direction)        const;
    uint8_t                                             toControlerSpin(unsigned int spin)                  const;
    uint8_t                                             toControlerColorMode(unsigned int color_mode)       const;
    uint8_t                                             toControlerProfile(unsigned int profile)            const;
    uint8_t                                             toControlerBrightness(unsigned int brightness)      const;

protected:

    std::unique_ptr<LenovoUSBController>  controller;
};

}
