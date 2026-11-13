// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "LenovoRGBController.h"

#include <KitLog.h>

#include <QRandomGenerator>
#include <QDeadlineTimer>

#include <algorithm>
#include <optional>
#include <set>
#include <thread>
#include <tuple>
#include <vector>



namespace LenovoLegionDaemon::Kit {

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

    /*
     * Wait until the keyboard switches the profile. Deadline is measured in real time,
     * each check is a HID round trip and the daemon event loop is blocked meanwhile.
     */
    auto waitForApplyProfileOnController = [this](std::chrono::milliseconds maxWaitTime = std::chrono::milliseconds(1000)) {
        const QDeadlineTimer deadline(maxWaitTime);

        while(fromControlerProfile(controller->getCurrentProfileId()) != m_profiles.active)
        {
            if(deadline.hasExpired())
            {
                LOG_W(QString(__PRETTY_FUNCTION__) + QString(": Timeout waiting for profile switch to profile %1").arg(m_profiles.active));
                break;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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
            if(const auto profile = fromControlerProfile(controller->getCurrentProfileId()))
            {
                m_profiles.active = *profile;
            }

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

        if(const auto profile = fromControlerProfile(controller->getCurrentProfileId()))
        {
            m_profiles.active = *profile;
        }
        else
        {
            LOG_W(QString(__PRETTY_FUNCTION__) + QString(":  No valid profile read, profile %1 is kept").arg(m_profiles.active));
        }
    }
}

/*
 * The controller applies a written value later: a read right after the write gives the previous value (with the right
 * operation, the retries of sendAndGetFeatureReport do not help). The value is read until the controller confirms it,
 * so a read after the write (the GUI reads the data again at once) gets the new value.
 *
 * check() reads the device and returns true when the written state is confirmed, at most ~420 ms.
 */
template<class Check>
static bool waitUntil(Check&& check)
{
    constexpr int ATTEMPTS = 6;

    for(int attempt = 0; attempt < ATTEMPTS; ++attempt)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(20 * (attempt + 1)));

        if(check())
        {
            return true;
        }
    }

    return false;
}

/*
 * read() returns an empty value for an invalid answer: not a confirmation, the next attempt follows
 */
template<class Read,class Value>
static bool waitForValue(Read&& read,const Value& expected,std::optional<Value>& last)
{
    return waitUntil([&] { last = read(); return last == expected; });
}

template<class Value>
static QString toString(const std::optional<Value>& value)
{
    return value.has_value() ? QString::number(static_cast<int>(*value)) : QString("invalid answer");
}

void LenovoRGBController::DeviceUpdateBrightness()
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ": Updating brightness on device");

    const uint8_t          brightness = toControlerBrightness(static_cast<uint8_t>(m_britnesses.active));
    std::optional<uint8_t> current;

    controller->setBrightness(brightness);

    if(!waitForValue([this] { return controller->getCurrentBrightness(); },brightness,current))
    {
        LOG_W(QString(__PRETTY_FUNCTION__) + QString(":  Brightness %1 was not confirmed by the controller, it reads %2").arg(static_cast<int>(brightness)).arg(toString(current)));
    }
}

void LenovoRGBController::DeviceRefreshBrightness(bool waitForChange)
{
    LOG_T(QString(__PRETTY_FUNCTION__) + ":  Refreshing brightness from device");

    /*
     * Changed by the keyboard: a read right after the event can give the previous brightness (see waitUntil),
     * the last read stays if it does not change (e.g. the same level again)
     */
    if(waitForChange)
    {
        const unsigned int before = m_britnesses.active;

        if(!waitUntil([&] {
                const auto brightness = fromControlerBrightness(controller->getCurrentBrightness());

                if(!brightness.has_value())
                {
                    return false;   // invalid answer is not a change
                }

                m_britnesses.active = *brightness;
                return m_britnesses.active != before;
            }))
        {
            LOG_D(QString(__PRETTY_FUNCTION__) + QString(":  Brightness did not change after the event, it reads %1").arg(m_britnesses.active));
        }

        return;
    }

    if(const auto brightness = fromControlerBrightness(controller->getCurrentBrightness()))
    {
        m_britnesses.active = *brightness;
    }
    else
    {
        LOG_W(QString(__PRETTY_FUNCTION__) + QString(":  No valid brightness read, brightness %1 is kept").arg(m_britnesses.active));
    }
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
        const bool          logoState = m_logoState;
        std::optional<bool> current;

        controller->setLogoStatus(logoState);

        if(!waitForValue([this] { return controller->getLogoStatus(); },logoState,current))
        {
            LOG_W(QString(__PRETTY_FUNCTION__) + QString(":  Logo state %1 was not confirmed by the controller, it reads %2").arg(logoState ? "on" : "off").arg(current.has_value() ? (*current ? "on" : "off") : "invalid answer"));
        }
    }
}

void LenovoRGBController::DeviceRefreshLogoState()
{
    if(m_hasLogo)
    {
        if(const auto logoState = controller->getLogoStatus())
        {
            m_logoState = *logoState;
        }
        else
        {
            LOG_W(QString(__PRETTY_FUNCTION__) + QString(":  No valid logo state read, logo state %1 is kept").arg(m_logoState ? "on" : "off"));
        }
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

    /*
     * Not accepted (too large): the effects of the device are read again, the client gets the error
     */
    try {
        controller->setProfileDescription(toControlerProfile(m_profiles.active), groups);
    }
    catch(...)
    {
        DeviceRefreshEffects();
        throw;
    }

    /*
     * A keyboard can keep only a part of the effects and silently drop the rest (c197: about 481 bytes, whole effects or
     * LEDs at the end, found by tests): the effects are read back and compared, the client gets an error instead of
     * a silently shortened list (all models). The effects read back stay (they are what the keyboard shows).
     */
    size_t sentEffects = groups.size(), sentLeds = 0;

    for(const auto& group : groups)
    {
        sentLeds += group.m_leds.size();
    }

    size_t keptEffects = 0, keptLeds = 0;

    /*
     * A read right after the write can give the effects before the write (old answer of the same operation):
     * read again until the effects match the written ones (mode and LEDs of every effect, the colors may be changed
     * by the keyboard), a real shortened list stays after the last attempt
     */
    waitUntil([&] {
        DeviceRefreshEffects();

        keptEffects = m_effects.size();
        keptLeds    = 0;

        bool same = keptEffects == sentEffects;

        for(size_t i = 0; i < m_effects.size(); ++i)
        {
            std::set<unsigned int> codes;

            for(const auto& led : m_effects[i].m_leds)
            {
                codes.insert(led.value);
            }

            keptLeds += codes.size();

            same = same && m_effects[i].m_mode == groups[i].m_mode
                        && std::equal(codes.begin(),codes.end(),groups[i].m_leds.begin(),groups[i].m_leds.end());
        }

        return same && keptLeds == sentLeds;
    });

    if(keptEffects != sentEffects || keptLeds != sentLeds)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::EFFECTS_NOT_KEPT,
                        std::string("The keyboard kept only ").append(std::to_string(keptEffects)).append(" of ").append(std::to_string(sentEffects))
                        .append(" effects and ").append(std::to_string(keptLeds)).append(" of ").append(std::to_string(sentLeds))
                        .append(" LEDs: the memory of the keyboard is full (fewer effects, LEDs or colors)"));
    }
}

void LenovoRGBController::DeviceResetEffectsToDefault()
{
    /*
     * Effects (mode, LEDs, colors) for a comparison
     */
    auto effects = [this] {
        std::vector<std::tuple<unsigned int,std::set<unsigned int>,std::vector<RGBColor>>> result;

        for(const auto& effect : m_effects)
        {
            std::set<unsigned int> codes;

            for(const auto& led : effect.m_leds)
            {
                codes.insert(led.value);
            }

            result.emplace_back(effect.m_mode,codes,effect.m_colors);
        }

        return result;
    };

    const auto before = effects();

    controller->setProfileDefault(toControlerProfile(m_profiles.active));

    /*
     * A read right after the reset can give the effects before it (old answer of the same operation): read until
     * they change, the effects which were already the default ones do not change (the last read stays)
     */
    waitUntil([&] {
        DeviceRefreshEffects();
        return effects() != before;
    });
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

std::optional<unsigned int> LenovoRGBController::fromControlerBrightness(std::optional<uint8_t> brightness) const
{
    if(!brightness.has_value())
    {
        return std::nullopt;
    }

    if(*brightness < m_britnesses.min || *brightness > m_britnesses.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + QString(":  Received brightness level %1 from controller is out of range").arg(static_cast<int>(*brightness)));
        return std::nullopt;
    }

    return *brightness;
}

std::optional<unsigned int> LenovoRGBController::fromControlerProfile(std::optional<uint8_t> profile) const
{
    if(!profile.has_value())
    {
        return std::nullopt;
    }

    if(*profile < m_profiles.min || *profile > m_profiles.max)
    {
        LOG_E(QString(__PRETTY_FUNCTION__) + QString(":  Received profile id %1 from controller is out of range").arg(static_cast<int>(*profile)));
        return std::nullopt;
    }

    return *profile;
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
