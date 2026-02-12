// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "LenovoRGBController.h"

#include <Core/LoggerHolder.h>

#include <QRandomGenerator>

#include <thread>



namespace LenovoLegionDaemon {

LenovoRGBController::LenovoRGBController(LenovoUSBController* controller_ptr,
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
                                         ) :
    RGBController( profiles,britnesses,name,vendor,description,serial,location,type,modes,zones,maxEffects,hasLogo),
    controller(controller_ptr)
{}

LenovoRGBController::~LenovoRGBController()
{}


void LenovoRGBController::DeviceUpdateProfile()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ": Updating active profile on device");

    auto waitForApplyProfileOnController = [this](int maxWaitTimeinMicros = 1000000) {
        while(fromControlerProfile(controller->getCurrentProfileId()) != m_profiles.active)
        {
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            maxWaitTimeinMicros -= 100;

            if(maxWaitTimeinMicros <= 0)
            {
                break;
            }
        }
    };

    controller->setProfile(toControlerProfile(m_profiles.active));

    waitForApplyProfileOnController();

    DeviceRefreshEffects();
}

void LenovoRGBController::DeviceRefreshProfile(int expectedProfile)
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Refreshing active profile from device");
    
    if(expectedProfile >= static_cast<int>(m_profiles.min) && expectedProfile <=  static_cast<int>(m_profiles.max))
    {
        LOG_D(QString(__PRETTY_FUNCTION__) + ":  Expecting profile " + QString::number(expectedProfile) + " after refresh");

        // Wait for USB device to switch to expected profile
        int atempt = 0;
        do {
            m_profiles.active = fromControlerProfile(controller->getCurrentProfileId());

            if(m_profiles.active  == static_cast<unsigned int>(expectedProfile))
            {
                LOG_D(QString(__PRETTY_FUNCTION__) + QString(":  Successfully switched to expected profile %1").arg(m_profiles.active));
                break;
            }

            if(atempt > 10)
            {
                LOG_W( __PRETTY_FUNCTION__ + QString(":  Timeout waiting for profile switch to profile %1").arg(m_profiles.active));
                break;
            }

            LOG_D(QString(__PRETTY_FUNCTION__) + QString(":  Waiting for profile switch to expected profile %1, currently at profile %2").arg(expectedProfile).arg(m_profiles.active));

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            atempt += 1;
        } while (true);
    }else
    {
        LOG_T(QString(__PRETTY_FUNCTION__) + ":  No expected profile provided, reading current profile from device");

        m_profiles.active = fromControlerProfile(controller->getCurrentProfileId());
    }
}

void LenovoRGBController::DeviceUpdateBrightness()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ": Updating brightness on device");
    controller->setBrightness(toControlerBrightness(static_cast<uint8_t>(m_britnesses.active)));
}

void LenovoRGBController::DeviceRefreshBrightness()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Refreshing brightness from device");
    m_britnesses.active = fromControlerBrightness(controller->getCurrentBrightness());
}

std::vector<RGBColor> LenovoRGBController::DeviceGetState() const
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Getting current LED state from device");

    std::vector<RGBColor> colors;

    auto state = controller->getState();

    for (const auto & led : m_leds)
    {
        if(state.find(led.value) != state.end())
        {
            colors.push_back(state.at(led.value));
        }
        else
        {
            colors.push_back(0x0);
        }
    }

    return colors;
}

uint16_t LenovoRGBController::GetVendorID() const
{
    return controller->getVid();
}

uint16_t LenovoRGBController::GetProductID() const
{
    return controller->getPid();
}

void LenovoRGBController::DeviceUpdateLogoState()
{
    if(m_hasLogo)
    {
        controller->setLogoStatus(m_logoState);
    }
}

void LenovoRGBController::DeviceRefreshLogoState()
{
    if(m_hasLogo)
    {
        m_logoState = controller->getLogoStatus();
    }
}

void LenovoRGBController::DeviceRefresh(int expectedProfile)
{
    /*
     * Read active profile
     */
    DeviceRefreshProfile(expectedProfile);

    /*
     * Read brightness
     */
    DeviceRefreshBrightness();

    /*
     * Read active profile settings
     */
    DeviceRefreshEffects();
}

void LenovoRGBController::DeviceUpdateEfects()
{
    std::vector<LenovoUSBController::led_group> groups;

    for (auto & effect : m_effects)
    {
        groups.push_back({
            .m_mode         = static_cast<uint8_t>(effect.m_mode),
            .m_speed        = static_cast<uint8_t>(effect.m_speed),
            .m_spin         = toControlerSpin(effect.m_direction),
            .m_direction    = toControlerDirection(effect.m_direction),
            .m_color_mode   = toControlerColorMode(effect.m_color_mode),
            .m_colors       = effect.m_colors,
            .m_leds         = [&effect]() {
                                std::set<uint16_t> leds;
                                for(const auto& led: effect.m_leds)
                                {
                                    leds.insert(led.value);
                                }
                                return std::vector<uint16_t>(leds.begin(), leds.end());
                            }()
        });
    }

    controller->setProfileDescription(toControlerProfile(m_profiles.active), groups);
}

void LenovoRGBController::DeviceResetEffectsToDefault()
{
    controller->setProfileDefault(toControlerProfile(m_profiles.active));
}

void LenovoRGBController::DeviceRefreshEffects()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Refreshing effects from device");

    readActiveProfileSettings();
}

void LenovoRGBController::readActiveProfileSettings()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Reading active profile settings from device");

    m_effects.clear();

    /*---------------------------------------------------------*\
    | Retrieve current values by readingledIdToIndex the device |
    \*---------------------------------------------------------*/
    std::vector<LenovoUSBController::led_group> l_currentSettings = controller->getProfileDescription(toControlerProfile(m_profiles.active));


    for (const auto & grub: l_currentSettings)
    {
        m_effects.push_back(led_group_effect{
            .m_mode        = grub.m_mode,
            .m_speed       = grub.m_speed,
            .m_direction   = fromControlerDirection(grub.m_direction) == MODE_DIRECTION_NA ? fromControlerSpin(grub.m_spin) : fromControlerDirection(grub.m_direction),
            .m_color_mode  = fromControlerColorMode(m_modes [std::distance(m_modes.begin(),std::find_if(m_modes.begin(),m_modes.end(),[&grub](const mode& m){ return m.value == grub.m_mode;})) ].flags, grub.m_color_mode),
            .m_colors      = grub.m_colors,
            .m_leds        = [this,&grub](){
                                std::vector<led> leds;

                                std::multimap<unsigned int,size_t>     l_ledIdToIndex;

                                /*
                                 * Build the led id to index map
                                 */
                                for (size_t idx = 0; idx < this->m_leds.size(); ++idx) {
                                    LOG_T(QString::asprintf("Mapping LED Index %zu to ID %X", idx, this->m_leds.at(idx).value));
                                    l_ledIdToIndex.insert(std::pair(this->m_leds.at(idx).value,idx));
                                }

                                for (uint16_t ledId : grub.m_leds)
                                {
                                    LOG_T(QString::asprintf("Mapping LED ID %X to colors", ledId));
                                    auto range = l_ledIdToIndex.equal_range(ledId);

                                    for (auto i = range.first; i != range.second; ++i)
                                    {
                                        LOG_T(QString::asprintf(" - Setting color for LED Index %lu", i->second));
                                        leds.push_back(this->m_leds.at(i->second));
                                    }
                                }

                                return leds;
                            }()
        });
    }
}

unsigned int LenovoRGBController::fromControlerColorMode(const unsigned int  modeFlags,const uint8_t color_mode) const
{
    switch(color_mode)
    {
    case 0x02:
    {
        if(modeFlags & MODE_FLAG_HAS_PER_LED_COLOR)
        {
            return  MODE_COLORS_PER_LED;
        }
        else if(modeFlags & MODE_FLAG_HAS_MODE_SPECIFIC_COLOR)
        {
            return MODE_COLORS_MODE_SPECIFIC;
        }
    }
    break;
    case 0x01:
    {
        if(modeFlags & MODE_FLAG_HAS_RANDOM_COLOR)
        {
            return MODE_COLORS_RANDOM;
        }
    }
    break;
    default:
        return MODE_COLORS_NONE;
    }

    return MODE_COLORS_NONE;
}

unsigned int LenovoRGBController::fromControlerDirection(const uint8_t direction) const
{
    switch(direction)
    {
    case 0x01:
        return MODE_DIRECTION_UP;
        break;
    case 0x02:
        return MODE_DIRECTION_DOWN;
        break;
    case 0x03:
        return MODE_DIRECTION_LEFT;
        break;
    case 0x04:
        return MODE_DIRECTION_RIGHT;
        break;
    }

    return MODE_DIRECTION_NA;
}

unsigned int LenovoRGBController::fromControlerSpin(const uint8_t spin) const
{
    switch(spin)
    {
    case 0x01:
        return MODE_DIRECTION_SPINLEFT;
        break;
    case 0x02:
        return MODE_DIRECTION_SPINRIGHT;
        break;
    }

    return MODE_DIRECTION_NA;
}

unsigned int LenovoRGBController::fromControlerBrightness(uint8_t brightness) const
{
    if(brightness < m_britnesses.min || brightness > m_britnesses.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + ":  Received brightness level from controller is out of range");
        return m_britnesses.min;
    }

    return static_cast<uint8_t>(brightness);
}

unsigned int LenovoRGBController::fromControlerProfile(uint8_t profile) const
{
    if(profile < m_profiles.min || profile > m_profiles.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + ":  Received profile id from controller is out of range");
        return m_profiles.min;
    }

    return static_cast<uint8_t>(profile);
}

uint8_t LenovoRGBController::toControlerDirection(unsigned int  direction) const
{
    switch(direction)
    {
    case MODE_DIRECTION_UP:
        return 0x01;
        break;

    case MODE_DIRECTION_DOWN:
        return 0x02;
        break;

    case MODE_DIRECTION_LEFT:
        return 0x03;
        break;

    case MODE_DIRECTION_RIGHT:
        return 0x04;
        break;
    default:
        return 0x00;
        break;
    }
}

uint8_t LenovoRGBController::toControlerSpin(unsigned int spin) const
{
    switch(spin)
    {
    case MODE_DIRECTION_SPINRIGHT:
        return 0x02;
        break;

    case MODE_DIRECTION_SPINLEFT:
        return 0x01;
        break;
    default:
        return 0x00;
        break;
    }
}

uint8_t LenovoRGBController::toControlerColorMode(unsigned int color_mode) const
{
    switch(color_mode)
    {
    default:
    case MODE_COLORS_NONE:
        return 0x00;
        break;

    case MODE_COLORS_RANDOM:
        return 0x01;
        break;

    case MODE_COLORS_MODE_SPECIFIC:
    case MODE_COLORS_PER_LED:
        return 0x02;
        break;
    }
}

uint8_t LenovoRGBController::toControlerBrightness(unsigned int brightness) const
{
    if(brightness < m_britnesses.min || brightness > m_britnesses.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + ":  Trying to set brightness level to controller which is out of range");
        return static_cast<uint8_t>(m_britnesses.min);
    }

    return static_cast<uint8_t>(brightness);
}

uint8_t LenovoRGBController::toControlerProfile(unsigned int profile) const
{
    if(profile < m_profiles.min || profile > m_profiles.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + ":  Trying to set profile id to controller which is out of range");
        return static_cast<uint8_t>(m_profiles.min);
    }

    return static_cast<uint8_t>(profile);
}

}
