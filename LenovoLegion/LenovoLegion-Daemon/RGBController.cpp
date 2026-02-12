// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include <cstring>

#include "RGBController.h"

namespace LenovoLegionDaemon {

RGBController::RGBController(const Profiles&            profiles,
                             const Brightnesses&        britnesses,
                             const std::string&         name,
                             const std::string&         vendor,
                             const std::string&         description,
                             const std::string&         serial,
                             const std::string&         location,
                             device_type                type,
                             const std::vector<mode>    &modes,
                             const std::vector<zone>    &zones,
                             unsigned int               maxEffects,
                             bool hasLogo) :
    m_profiles(profiles)      ,
    m_britnesses(britnesses)  ,
    m_type(type)              ,
    m_name(name)              ,
    m_vendor(vendor)          ,
    m_description(description),
    m_serial(serial)          ,
    m_location(location)      ,
    m_modes(modes)            ,
    m_zones(zones)            ,
    m_leds
    (
        [] (const std::vector<zone> &zones) {
            std::vector<led> leds;
            for (const zone &z : zones) {
                leds.insert(leds.end(), z.leds.begin(), z.leds.end());
            }
            return leds;
        }(zones)
    ),
    m_maxEffects(maxEffects),
    m_hasLogo(hasLogo),
    m_logoState(false)
{}

RGBController::~RGBController()
{}

std::string RGBController::GetName() const
{
    return m_name;
}

std::string RGBController::GetVendor() const
{
    return m_vendor;
}

std::string RGBController::GetDescription() const
{
    return m_description;
}

std::string RGBController::GetSerial() const
{
    return m_serial;
}

std::string RGBController::GetLocation() const
{
    return m_location;
}

std::string RGBController::GetModeNameByIdx(unsigned int mode) const
{
    return m_modes.at(mode).name;
}

mode RGBController::GetModeByModeValue(int mode) const
{
    for (const auto & m : m_modes)
    {
        if(m.value == mode)
        {
            return m;
        }
    }

    throw std::out_of_range("Mode value not found");
}

mode RGBController::GetModeByIdx(unsigned int mode) const
{
    return m_modes.at(mode);
}

std::string RGBController::GetZoneName(unsigned int zone) const
{
    return m_zones.at(zone).name;
}

std::string RGBController::GetLEDName(unsigned int led) const
{
    return m_leds.at(led).name;
}


unsigned int RGBController::GetLEDsInZone(unsigned int zone) const
{
    return m_zones.at(zone).leds_count;
}

void RGBController::SetProfile(unsigned int profileIdx)
{
    m_profiles.active = profileIdx;
}


const std::vector<led>& RGBController::GetLEDs() const
{
    return m_leds;
}


const std::vector<zone>& RGBController::GetZones() const
{
    return m_zones;
}

std::string RGBController::GetControllerName() const
{
    return m_name;
}

const std::vector<mode>& RGBController::GetModes() const
{
    return m_modes;
}


device_type RGBController::GetDeviceType() const
{
    return m_type;
}

const std::vector<led_group_effect> &RGBController::GetEffects() const
{
    return m_effects;
}

unsigned int RGBController::GetMaxEffects() const
{
    return m_maxEffects;
}

const led_group_effect &RGBController::GetEffect(unsigned int effectIdx) const
{
    return m_effects.at(effectIdx);
}

void RGBController::SetEfects(const std::vector<led_group_effect> &effects)
{
    m_effects = effects;
}

void RGBController::AddEffect(const led_group_effect &effect)
{
    m_effects.push_back(effect);
}

void RGBController::RemoveEffect(unsigned int effectIdx)
{
    m_effects.erase(m_effects.begin() + effectIdx);
}

void RGBController::ClearEffects()
{
    m_effects.clear();
}

void RGBController::ResetEffectsToDefault()
{
    DeviceResetEffectsToDefault();
}

std::set<int> RGBController::GetLedsIndexesByDeviceSpecificValue(unsigned int value) const
{
    std::set<int> indices;

    for(std::size_t i = 0; i < m_leds.size(); i++)
    {
        if(m_leds[i].value == value)
        {
            indices.insert((int)i);
        }
    }

    return indices;
}

std::vector<RGBColor> RGBController::GetStateForAllLeds() const
{
    return DeviceGetState();
}

bool RGBController::HasLogo() const
{
    return m_hasLogo;
}

bool RGBController::GetLogoState() const
{
    return m_logoState;
}

void RGBController::SetLogoState(bool on)
{
    m_logoState = on;
}

const Profiles &RGBController::GetProfiles() const
{
    return m_profiles;
}

const Brightnesses& RGBController::GetBrightness() const
{
    return m_britnesses;
}

void RGBController::SetBrightness(unsigned int brightness)
{
    m_britnesses.active = brightness;
}

std::string device_type_to_str(device_type type)
{
    switch(type)
    {
    case DEVICE_TYPE_KEYBOARD:
        return "Keyboard";
    default:
        return "Unknown";
    }
}

}
