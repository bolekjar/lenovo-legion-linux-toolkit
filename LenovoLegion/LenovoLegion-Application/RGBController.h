// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <Core/ExceptionBuilder.h>

#include "../LenovoLegion-Daemon/RGBControllerInterface.h"

#include <bitset>

namespace LenovoLegionGui {

class DataProvider;

class RGBController : public LenovoLegionDaemon::RGBControllerInterface
{
    enum PENDING_CHANGES : int {
        CHANGE_PROFILES         = 0,
        CHANGE_BRIGHTNESS       = 1,
        CHANGE_EFFECTS          = 2,
        CHANGE_RESET_EFFECTS    = 3,
        CHANGE_LOGO_STATE       = 4,
        MAXIMUM_CHANGES         = 5
    };

public:
    DEFINE_EXCEPTION(RGBController)

    enum ERROR_CODES : int {
        DATA_NOT_AVAILABLE = -1,
        INVALID_INDEX = -2
    };

public:
    explicit RGBController(DataProvider* dataProvider);
    virtual ~RGBController() override;

    /*
     * Device Type
     */
    virtual LenovoLegionDaemon::device_type                   GetDeviceType()      const                        override;


    /*
     * Profiles
     */
    virtual const LenovoLegionDaemon::Profiles&         GetProfiles()                          const            override;
    virtual void                                        SetProfile(unsigned int profileIdx)                     override;


    /*
     * Zones
     */
    virtual const std::vector<LenovoLegionDaemon::zone>&      GetZones()    const                              override;
    virtual unsigned int    GetLEDsInZone(unsigned int zone)                const                              override;
    virtual std::string     GetZoneName(unsigned int zone)                  const                              override;

    /*
     * Brightnesses
     */
    virtual const LenovoLegionDaemon::Brightnesses&     GetBrightness()      const                             override;
    virtual void                                        SetBrightness(unsigned int brightness)                 override;



    /*
     * Effects
     */
    virtual const std::vector<LenovoLegionDaemon::led_group_effect>&   GetEffects()                         const                                   override;
    virtual unsigned int                                               GetMaxEffects()                      const                                   override;
    virtual const LenovoLegionDaemon::led_group_effect&                GetEffect(unsigned int effectIdx)    const                                   override;
    virtual void                                                       SetEfects(const std::vector<LenovoLegionDaemon::led_group_effect>& effects)  override;
    virtual void                                                       AddEffect(const LenovoLegionDaemon::led_group_effect& effect)                override;
    virtual void                                                       RemoveEffect(unsigned int effectIdx)                                         override;
    virtual void                                                       ClearEffects()                                                               override;
    virtual void                                                       ResetEffectsToDefault()                                                      override;


    /*
     * Modes
     */
    virtual const std::vector<LenovoLegionDaemon::mode>&      GetModes()                                const                        override;
    virtual std::string                                       GetModeNameByIdx(unsigned int mode)       const                        override;
    virtual LenovoLegionDaemon::mode                          GetModeByIdx(unsigned int mode)           const                        override;
    virtual LenovoLegionDaemon::mode                          GetModeByModeValue(int mode)              const                        override;


    /*
     * Leds maping information
     */
    virtual const std::vector<LenovoLegionDaemon::led>&       GetLEDs()                         const              override;
    virtual std::string                                       GetLEDName(unsigned int led)      const              override;
    virtual std::set<int>              GetLedsIndexesByDeviceSpecificValue(unsigned int value)  const              override;


    /*
     * Current states of leds
     */
    virtual std::vector<LenovoLegionDaemon::RGBColor>         GetStateForAllLeds()              const              override;


    virtual bool                                        HasLogo()                         const              override;
    virtual bool                                        GetLogoState()                    const              override;
    virtual void                                        SetLogoState(bool on)                                override;


    /*
     * Apply pending changes
     */
    void ApplyPendingChanges();

    /*
     * Refresh data from daemon
     */
    void RefreshData();


    /*
     * Device Information
     */
    std::string name()          const;
    std::string vendor()        const;
    std::string location()      const;
    std::string serial()        const;
    std::string description()   const;
    uint32_t vendorId()         const;
    uint32_t productId()        const;

private:
    void readRGBControllerData(const uint32_t requestFlags);
    void sendRGBControllerData();

private:
    DataProvider* m_dataProvider;

    std::vector<LenovoLegionDaemon::led>        m_leds;
    std::vector<LenovoLegionDaemon::zone>       m_zones;
    std::vector<LenovoLegionDaemon::mode>       m_modes;

    LenovoLegionDaemon::device_type   m_deviceType    = LenovoLegionDaemon::DEVICE_TYPE_UNKNOWN;

    LenovoLegionDaemon::Profiles                        m_profiles;
    LenovoLegionDaemon::Brightnesses                    m_brightness;
    unsigned int                                        m_maxEffects;

    std::vector<LenovoLegionDaemon::led_group_effect>   m_effects;

    std::bitset<MAXIMUM_CHANGES>            m_pendingChanges;

    /*
     * Device information
     */
    std::string   m_name;
    std::string   m_vendor;
    std::string   m_location;
    std::string   m_serial;
    std::string   m_description;

    uint32_t      m_vendorId   = 0;
    uint32_t      m_productId  = 0;

    bool m_hasLogo     = false;
    bool m_logoState   = false;
};

}
