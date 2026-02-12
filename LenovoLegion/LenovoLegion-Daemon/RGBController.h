// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "RGBControllerInterface.h"

namespace LenovoLegionDaemon {

class RGBController : public QObject,
                      public RGBControllerInterface
{
    Q_OBJECT

public:
    /*---------------------------------------------------------*\
    | RGBController base class constructor                      |
    \*---------------------------------------------------------*/
    RGBController(const Profiles&                profiles,
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

    virtual ~RGBController();

    /*---------------------------------------------------------*\
    | Generic functions implemented in RGBController.cpp        |
    \*---------------------------------------------------------*/
    std::string             GetName()           const;
    std::string             GetVendor()         const;
    std::string             GetDescription()    const;
    std::string             GetSerial()         const;
    std::string             GetLocation()       const;
    std::string             GetControllerName() const;
public:

    /*---------------------------------------------------------*\
    | Functions From RGBControllerInterface      |
    \*---------------------------------------------------------*/

    /*
     * Device Type
     */
    virtual device_type                     GetDeviceType()  const                    override;

    /*
     * Profiles Information
     */
    virtual const Profiles&                 GetProfiles()         const               override;
    virtual void                            SetProfile(unsigned int profileIdx)       override;


    /*
     * Zone Information
     */
    virtual const std::vector<zone>&        GetZones()                       const   override;
    virtual std::string                     GetZoneName(unsigned int zone)   const   override;
    virtual unsigned int                    GetLEDsInZone(unsigned int zone) const   override;


    /*
     * Britness Information
     */
    virtual const Brightnesses&             GetBrightness()       const               override;
    virtual void                            SetBrightness(unsigned int brightness)    override;


    /*
     * Effects Information
     */
    virtual const
        std::vector<led_group_effect>&    GetEffects()       const                                override;
    virtual unsigned int                  GetMaxEffects()    const                                override;
    virtual const led_group_effect&       GetEffect(unsigned int effectIdx)  const                override;
    virtual void                          SetEfects(const std::vector<led_group_effect>& effects) override;
    virtual void                          AddEffect(const led_group_effect& effect)               override;
    virtual void                          RemoveEffect(unsigned int effectIdx)                    override;
    virtual void                          ClearEffects()                                          override;
    virtual void                          ResetEffectsToDefault()                                 override;


    /*
     * Modes
     */
    virtual const std::vector<LenovoLegionDaemon::mode>&      GetModes()           const          override;
    virtual std::string     GetModeNameByIdx(unsigned int mode)                    const          override;
    virtual LenovoLegionDaemon::mode   GetModeByIdx(unsigned int mode)             const          override;
    virtual mode                          GetModeByModeValue(int mode)             const          override;


    /*
     * Led Maping information
     */
    virtual const std::vector<led>&       GetLEDs()                                               const  override;
    virtual std::string                   GetLEDName(unsigned int led)                            const  override;
    virtual std::set<int>                 GetLedsIndexesByDeviceSpecificValue(unsigned int value) const  override;

    /*
     * Get current colors for all LEDs
     */
    virtual std::vector<RGBColor>         GetStateForAllLeds()                                    const override;


    /*
     * Logo Information
     */
    virtual bool                          HasLogo()                                       const   override;
    virtual bool                          GetLogoState()                                  const   override;
    virtual void                          SetLogoState(bool on)                                   override;

public:

    /*---------------------------------------------------------*\
    | Functions to be implemented in device implementation      |
    \*---------------------------------------------------------*/
    virtual void        DeviceUpdateEfects()                    =   0;
    virtual void        DeviceResetEffectsToDefault()           =   0;
    virtual void        DeviceRefreshEffects()                  =   0;

    virtual void        DeviceUpdateProfile()                   =   0;
    virtual void        DeviceRefreshProfile(int expectedProfile = -1)  =   0;

    virtual void        DeviceUpdateBrightness()                =   0;
    virtual void        DeviceRefreshBrightness()               =   0;

    virtual void        DeviceRefresh(int expectedProfile = -1) =   0;

    virtual std::vector<RGBColor>        DeviceGetState() const =   0;

    virtual uint16_t                GetVendorID()         const   = 0;
    virtual uint16_t                GetProductID()        const   = 0;

    virtual void                    DeviceUpdateLogoState()       = 0;
    virtual void                    DeviceRefreshLogoState()      = 0;

protected:

    Profiles             m_profiles         = {0,0,0};              /* Supported Device Profiles*/
    Brightnesses         m_britnesses       = {0,0,0};              /* Supported Brightness Levels*/
    const device_type    m_type             = DEVICE_TYPE_UNKNOWN;  /* device type              */
    const std::string    m_name;                                    /* controller name          */
    const std::string    m_vendor;                                  /* controller vendor        */
    const std::string    m_description;                             /* controller description   */
    const std::string    m_serial;                                  /* controller serial number */
    const std::string    m_location;                                /* controller location      */

    /*
     * Device Model Information
     */
    const std::vector<mode>       m_modes;            /* Supported Modes          */
    const std::vector<zone>       m_zones;            /* Supported Zones          */


    /*
     * Direct Leds control
     */
    const std::vector<led>         m_leds;           /* LEDs                     */

    /*
     * Profile effects
     */
    const unsigned int              m_maxEffects;    /* Max effects supported    */
    std::vector<led_group_effect>   m_effects;       /* Current effects          */

    /*
     * Logo
     */
    const bool                      m_hasLogo;       /* Whether the device has a logo zone */
          bool                      m_logoState;     /* Current state of the logo zone      */
};

}
