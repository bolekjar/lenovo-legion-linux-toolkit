// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderRGB4Zone.h"

#include <Drivers/SysFsDriverLegionGameZone.h>

#include <KitLog.h>

#include <hidapi.h>

#include <algorithm>
#include <array>

namespace LenovoLegionDaemon::Kit {

namespace {

constexpr unsigned short ITE_VID          = 0x048D;
constexpr unsigned short USAGE_PAGE       = 0xFF89;
constexpr unsigned short USAGE_4ZONE      = 0x00CC;

/*
 * Feature report 0xCC (LENOVO_RGB_KEYBOARD_STATE of Lenovo Legion Toolkit):
 *  0-1 header 0xCC 0x16, 2 effect, 3 speed 1-4, 4 brightness (0 off, 1 low, 2 high), 5-16 RGB of the zones 1-4,
 *  17 unused, 18 wave left to right, 19 wave right to left, 20-32 unused
 */
constexpr size_t  REPORT_SIZE         = 33;
constexpr uint8_t REPORT_ID           = 0xCC;
constexpr uint8_t REPORT_HEADER       = 0x16;
constexpr size_t  ZONES               = 4;

constexpr uint8_t EFFECT_STATIC       = 1;
constexpr uint8_t EFFECT_BREATH       = 3;
constexpr uint8_t EFFECT_WAVE         = 4;
constexpr uint8_t EFFECT_SMOOTH       = 6;

constexpr uint32_t SPEED_MIN          = 1;
constexpr uint32_t SPEED_MAX          = 4;

/*
 * State before the first set (the keyboard can not be read): static white, high brightness
 */
legion::messages::Keyboard4Zone defaultState()
{
    legion::messages::Keyboard4Zone state;

    state.set_present(true);
    state.set_effect(legion::messages::Keyboard4Zone::EFFECT_STATIC);
    state.set_speed(2);
    state.set_brightness(legion::messages::Keyboard4Zone::BRIGHTNESS_HIGH);
    state.set_direction(legion::messages::Keyboard4Zone::DIRECTION_LEFT_TO_RIGHT);

    state.set_zone1_color(0xFFFFFF);
    state.set_zone2_color(0xFFFFFF);
    state.set_zone3_color(0xFFFFFF);
    state.set_zone4_color(0xFFFFFF);

    return state;
}

std::array<uint8_t,REPORT_SIZE> report(const legion::messages::Keyboard4Zone& state)
{
    std::array<uint8_t,REPORT_SIZE> data {};

    data[0] = REPORT_ID;
    data[1] = REPORT_HEADER;

    if(state.effect() == legion::messages::Keyboard4Zone::EFFECT_OFF)
    {
        data[2] = EFFECT_STATIC;
        data[3] = static_cast<uint8_t>(SPEED_MIN);
        data[4] = 0;                                    // off: brightness 0, the colors black
        return data;
    }

    switch(state.effect())
    {
    case legion::messages::Keyboard4Zone::EFFECT_BREATH: data[2] = EFFECT_BREATH; break;
    case legion::messages::Keyboard4Zone::EFFECT_WAVE:   data[2] = EFFECT_WAVE;   break;
    case legion::messages::Keyboard4Zone::EFFECT_SMOOTH: data[2] = EFFECT_SMOOTH; break;
    default:                                             data[2] = EFFECT_STATIC; break;
    }

    data[3] = static_cast<uint8_t>(std::clamp(state.speed(),SPEED_MIN,SPEED_MAX));
    data[4] = state.brightness() == legion::messages::Keyboard4Zone::BRIGHTNESS_HIGH ? 2 : 1;

    const std::array<uint32_t,ZONES> colors { state.zone1_color(), state.zone2_color(), state.zone3_color(), state.zone4_color() };

    for(size_t zone = 0; zone < ZONES; ++zone)
    {
        const uint32_t color = colors[zone];

        data[5 + zone * 3]     = static_cast<uint8_t>((color >> 16) & 0xFF);
        data[5 + zone * 3 + 1] = static_cast<uint8_t>((color >> 8) & 0xFF);
        data[5 + zone * 3 + 2] = static_cast<uint8_t>(color & 0xFF);
    }

    if(state.effect() == legion::messages::Keyboard4Zone::EFFECT_WAVE)
    {
        const bool rightToLeft = state.direction() == legion::messages::Keyboard4Zone::DIRECTION_RIGHT_TO_LEFT;

        data[18] = rightToLeft ? 0 : 1;
        data[19] = rightToLeft ? 1 : 0;
    }

    return data;
}

}

DataProviderRGB4Zone::DataProviderRGB4Zone(SysFsDriverManager* sysFsDriverManager) :
    SysFsDataProvider(sysFsDriverManager,dataType),
    m_state(defaultState())
{}

std::string DataProviderRGB4Zone::find()
{
    std::string path;

    hid_device_info* devices = hid_enumerate(ITE_VID,0);

    for(const hid_device_info* device = devices; device != nullptr; device = device->next)
    {
        if(device->usage_page == USAGE_PAGE && device->usage == USAGE_4ZONE && device->path != nullptr)
        {
            path = device->path;
            LOG_D(QString::asprintf("4-zone RGB keyboard %04x:%04x found: %s",device->vendor_id,device->product_id,device->path));
            break;
        }
    }

    hid_free_enumeration(devices);

    return path;
}

void DataProviderRGB4Zone::init()
{
    clean();

    m_path = find();
}

void DataProviderRGB4Zone::clean()
{
    m_path.clear();
}

QByteArray DataProviderRGB4Zone::serializeAndGetData()
{
    LOG_T(__PRETTY_FUNCTION__);

    /*
     * Without a 4-zone keyboard an empty message: the data type is not available (available_from "present")
     */
    return serializeMessage(m_path.empty() ? legion::messages::Keyboard4Zone() : m_state);
}

QByteArray DataProviderRGB4Zone::deserializeAndSetData(const QByteArray &data)
{
    legion::messages::Keyboard4Zone request = parseMessage<legion::messages::Keyboard4Zone>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(request,Origin::REQUEST);

    if(m_path.empty())
    {
        return {};
    }

    /*
     * Fields not in the request keep the last state (the keyboard takes the whole state at once)
     */
    legion::messages::Keyboard4Zone state = m_state;

    if(request.has_effect())         state.set_effect(request.effect());
    if(request.has_speed())          state.set_speed(request.speed());
    if(request.has_brightness())     state.set_brightness(request.brightness());
    if(request.has_direction())      state.set_direction(request.direction());
    if(request.has_zone1_color())    state.set_zone1_color(request.zone1_color());
    if(request.has_zone2_color())    state.set_zone2_color(request.zone2_color());
    if(request.has_zone3_color())    state.set_zone3_color(request.zone3_color());
    if(request.has_zone4_color())    state.set_zone4_color(request.zone4_color());

    if(write(state))
    {
        m_state = state;
    }

    return {};
}

QByteArray DataProviderRGB4Zone::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<legion::messages::Keyboard4Zone>(data,[this,origin](legion::messages::Keyboard4Zone& keyboard) { settable(keyboard,origin); });
}

void DataProviderRGB4Zone::settable(legion::messages::Keyboard4Zone &keyboard, Origin origin) const
{
    keyboard.clear_present();

    if(m_path.empty())
    {
        if(keyboard.ByteSizeLong() > 0)
        {
            skipField(origin,"4-zone keyboard","no 4-zone RGB keyboard");
        }

        keyboard.Clear();
        return;
    }

    if(keyboard.has_speed() && (keyboard.speed() < SPEED_MIN || keyboard.speed() > SPEED_MAX))
    {
        skipField(origin,QString("4-zone keyboard speed=").append(QString::number(keyboard.speed())),"range <1,4>");
        keyboard.clear_speed();
    }

    auto checkColor = [&](bool has,uint32_t color,const char* name,auto clear) {
        if(has && color > 0xFFFFFF)
        {
            skipField(origin,QString("4-zone keyboard ").append(name).append("=0x").append(QString::number(color,16)),"0xRRGGBB");
            clear();
        }
    };

    checkColor(keyboard.has_zone1_color(),keyboard.zone1_color(),"zone1_color",[&] { keyboard.clear_zone1_color(); });
    checkColor(keyboard.has_zone2_color(),keyboard.zone2_color(),"zone2_color",[&] { keyboard.clear_zone2_color(); });
    checkColor(keyboard.has_zone3_color(),keyboard.zone3_color(),"zone3_color",[&] { keyboard.clear_zone3_color(); });
    checkColor(keyboard.has_zone4_color(),keyboard.zone4_color(),"zone4_color",[&] { keyboard.clear_zone4_color(); });
}

bool DataProviderRGB4Zone::write(const legion::messages::Keyboard4Zone &state)
{
    /*
     * The lights set by the application, not by the keyboard (Lenovo Legion Toolkit does the same before a write)
     */
    if(m_sysFsDriverManager->isLoaded(SysFsDriverLegionGameZone::DRIVER_NAME))
    {
        const SysFsDriverLegionGameZone::GameZone::Other other(m_sysFsDriverManager->getDriverDesriptor(SysFsDriverLegionGameZone::DRIVER_NAME));

        if(!other.set_light_control_owner.empty())
        {
            try {
                setData(other.set_light_control_owner,true);
            } catch(const std::exception& ex)
            {
                LOG_W(QString("4-zone RGB keyboard: light control owner not set: ").append(ex.what()));
            }
        }
    }

    hid_device* device = hid_open_path(m_path.c_str());

    if(device == nullptr)
    {
        LOG_W(QString("4-zone RGB keyboard: ").append(m_path.c_str()).append(" can not be opened"));
        return false;
    }

    const auto data   = report(state);
    const int  result = hid_send_feature_report(device,data.data(),data.size());

    if(result < 0)
    {
        LOG_W(QString("4-zone RGB keyboard: write failed: ").append(QString::fromWCharArray(hid_error(device))));
    }

    hid_close(device);

    return result >= 0;
}

}
