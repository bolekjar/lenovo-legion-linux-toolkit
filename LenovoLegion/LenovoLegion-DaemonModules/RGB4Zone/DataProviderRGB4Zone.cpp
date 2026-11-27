// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DataProviderRGB4Zone.h"

#include <Drivers/SysFsDriverLegionGameZone.h>

#include <HidReportDescriptor.h>
#include <KitLog.h>

#include <hidapi.h>

#include <algorithm>
#include <array>

namespace LenovoLegionDaemon::Kit {

namespace {

using Keyboard = legion::messages::Keyboard4Zone;

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

Keyboard::Preset preset(Keyboard::Effect effect,uint32_t zone1,uint32_t zone2,uint32_t zone3,uint32_t zone4)
{
    Keyboard::Preset result;

    result.set_effect(effect);
    result.set_speed(2);
    result.set_brightness(Keyboard::BRIGHTNESS_HIGH);
    result.set_direction(Keyboard::DIRECTION_LEFT_TO_RIGHT);
    result.set_zone1_color(zone1);
    result.set_zone2_color(zone2);
    result.set_zone3_color(zone3);
    result.set_zone4_color(zone4);

    return result;
}

/*
 * Presets before the first set (the keyboard can not be read, the same as Lenovo Legion Toolkit): static white, static
 * colors of the zones, wave, smooth; preset 1 active
 */
Keyboard defaultState()
{
    Keyboard state;

    state.set_present(true);
    state.set_active_preset(1);
    *state.add_presets() = preset(Keyboard::EFFECT_STATIC,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF);
    *state.add_presets() = preset(Keyboard::EFFECT_STATIC,0xFF0000,0x00FF00,0x0000FF,0x8000FF);
    *state.add_presets() = preset(Keyboard::EFFECT_WAVE,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF);
    *state.add_presets() = preset(Keyboard::EFFECT_SMOOTH,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF);

    return state;
}

std::array<uint8_t,REPORT_SIZE> report(uint32_t active,const Keyboard::Preset& preset)
{
    std::array<uint8_t,REPORT_SIZE> data {};

    data[0] = REPORT_ID;
    data[1] = REPORT_HEADER;

    /*
     * Off: brightness 0, the colors black
     */
    if(active == 0)
    {
        data[2] = EFFECT_STATIC;
        data[3] = static_cast<uint8_t>(SPEED_MIN);
        return data;
    }

    switch(preset.effect())
    {
    case Keyboard::EFFECT_BREATH: data[2] = EFFECT_BREATH; break;
    case Keyboard::EFFECT_WAVE:   data[2] = EFFECT_WAVE;   break;
    case Keyboard::EFFECT_SMOOTH: data[2] = EFFECT_SMOOTH; break;
    default:                      data[2] = EFFECT_STATIC; break;
    }

    data[3] = static_cast<uint8_t>(std::clamp(preset.speed(),SPEED_MIN,SPEED_MAX));
    data[4] = preset.brightness() == Keyboard::BRIGHTNESS_HIGH ? 2 : 1;

    const std::array<uint32_t,ZONES> colors { preset.zone1_color(), preset.zone2_color(), preset.zone3_color(), preset.zone4_color() };

    for(size_t zone = 0; zone < ZONES; ++zone)
    {
        data[5 + zone * 3]     = static_cast<uint8_t>((colors[zone] >> 16) & 0xFF);
        data[5 + zone * 3 + 1] = static_cast<uint8_t>((colors[zone] >> 8) & 0xFF);
        data[5 + zone * 3 + 2] = static_cast<uint8_t>(colors[zone] & 0xFF);
    }

    if(preset.effect() == Keyboard::EFFECT_WAVE)
    {
        const bool rightToLeft = preset.direction() == Keyboard::DIRECTION_RIGHT_TO_LEFT;

        data[18] = rightToLeft ? 0 : 1;
        data[19] = rightToLeft ? 1 : 0;
    }

    return data;
}

/*
 * Fields present in the request replace the ones of the preset
 */
void merge(Keyboard::Preset& preset,const Keyboard::Preset& request)
{
    if(request.has_effect())      preset.set_effect(request.effect());
    if(request.has_speed())       preset.set_speed(request.speed());
    if(request.has_brightness())  preset.set_brightness(request.brightness());
    if(request.has_direction())   preset.set_direction(request.direction());
    if(request.has_zone1_color()) preset.set_zone1_color(request.zone1_color());
    if(request.has_zone2_color()) preset.set_zone2_color(request.zone2_color());
    if(request.has_zone3_color()) preset.set_zone3_color(request.zone3_color());
    if(request.has_zone4_color()) preset.set_zone4_color(request.zone4_color());
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
        if(device->usage_page != USAGE_PAGE || device->usage != USAGE_4ZONE || device->path == nullptr)
        {
            continue;
        }

        /*
         * Only a collection whose descriptor has the feature report 0xCC of 33 bytes (the same check as the community
         * projects, not the product ID alone); without the descriptor (hidapi < 0.14) the collection is enough
         */
        hid_device* handle = hid_open_path(device->path);

        if(handle == nullptr)
        {
            LOG_W(QString::asprintf("4-zone RGB keyboard %04x:%04x: %s can not be opened",device->vendor_id,device->product_id,device->path));
            continue;
        }

        const auto length = hidFeatureReportLength(handle,REPORT_ID);

        hid_close(handle);

        if(length.has_value() && *length != REPORT_SIZE)
        {
            LOG_W(QString::asprintf("HID %04x:%04x collection 0x%04x:0x%04x is not a 4-zone keyboard (feature report 0x%02x: %zu bytes, 4-zone %zu bytes), skipped",
                                    device->vendor_id,device->product_id,device->usage_page,device->usage,REPORT_ID,*length,REPORT_SIZE));
            continue;
        }

        path = device->path;
        LOG_D(QString::asprintf("4-zone RGB keyboard %04x:%04x found: %s%s",device->vendor_id,device->product_id,device->path,
                                length.has_value() ? "" : " (report descriptor not available, hidapi < 0.14)"));
        break;
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
    return serializeMessage(m_path.empty() ? Keyboard() : m_state);
}

QByteArray DataProviderRGB4Zone::deserializeAndSetData(const QByteArray &data)
{
    Keyboard request = parseMessage<Keyboard>(data);

    LOG_T(__PRETTY_FUNCTION__);

    settable(request,Origin::REQUEST);

    if(m_path.empty())
    {
        return {};
    }

    Keyboard state = m_state;

    if(request.has_active_preset())
    {
        state.set_active_preset(request.active_preset());
    }

    for(int i = 0; i < request.presets_size() && i < PRESETS; ++i)
    {
        merge(*state.mutable_presets(i),request.presets(i));
    }

    const uint32_t active = state.active_preset();

    if(write(active,active == 0 ? Keyboard::Preset() : state.presets(static_cast<int>(active) - 1)))
    {
        m_state = state;
    }

    return {};
}

void DataProviderRGB4Zone::restore(const QByteArray &data)
{
    Keyboard request = parseMessage<Keyboard>(data);

    settable(request,Origin::REQUEST);

    if(request.has_active_preset())
    {
        m_state.set_active_preset(request.active_preset());
    }

    for(int i = 0; i < request.presets_size() && i < PRESETS; ++i)
    {
        merge(*m_state.mutable_presets(i),request.presets(i));
    }
}

bool DataProviderRGB4Zone::nextPreset()
{
    if(m_path.empty())
    {
        return false;
    }

    const uint32_t active = (m_state.active_preset() + 1) % (PRESETS + 1);

    LOG_D(QString("4-zone RGB keyboard: preset ").append(QString::number(active)).append(" (Fn+Space)"));

    if(!write(active,active == 0 ? Keyboard::Preset() : m_state.presets(static_cast<int>(active) - 1)))
    {
        return false;
    }

    m_state.set_active_preset(active);
    return true;
}

QByteArray DataProviderRGB4Zone::settableData(const QByteArray &data, Origin origin)
{
    return settableMessage<Keyboard>(data,[this,origin](Keyboard& keyboard) { settable(keyboard,origin); });
}

void DataProviderRGB4Zone::settable(Keyboard &keyboard, Origin origin) const
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

    if(keyboard.has_active_preset() && keyboard.active_preset() > static_cast<uint32_t>(PRESETS))
    {
        skipField(origin,QString("4-zone keyboard active_preset=").append(QString::number(keyboard.active_preset())),"range <0,4>");
        keyboard.clear_active_preset();
    }

    if(keyboard.presets_size() > PRESETS)
    {
        skipField(origin,QString("4-zone keyboard presets (").append(QString::number(keyboard.presets_size())).append(")"),"4 presets");
        keyboard.mutable_presets()->DeleteSubrange(PRESETS,keyboard.presets_size() - PRESETS);
    }

    for(int i = 0; i < keyboard.presets_size(); ++i)
    {
        Keyboard::Preset* preset = keyboard.mutable_presets(i);
        const QString     name   = QString("4-zone keyboard preset ").append(QString::number(i + 1)).append(' ');

        if(preset->has_speed() && (preset->speed() < SPEED_MIN || preset->speed() > SPEED_MAX))
        {
            skipField(origin,QString(name).append("speed=").append(QString::number(preset->speed())),"range <1,4>");
            preset->clear_speed();
        }

        auto checkColor = [&](bool has,uint32_t color,const char* field,auto clear) {
            if(has && color > 0xFFFFFF)
            {
                skipField(origin,QString(name).append(field).append("=0x").append(QString::number(color,16)),"0xRRGGBB");
                clear();
            }
        };

        checkColor(preset->has_zone1_color(),preset->zone1_color(),"zone1_color",[&] { preset->clear_zone1_color(); });
        checkColor(preset->has_zone2_color(),preset->zone2_color(),"zone2_color",[&] { preset->clear_zone2_color(); });
        checkColor(preset->has_zone3_color(),preset->zone3_color(),"zone3_color",[&] { preset->clear_zone3_color(); });
        checkColor(preset->has_zone4_color(),preset->zone4_color(),"zone4_color",[&] { preset->clear_zone4_color(); });
    }
}

bool DataProviderRGB4Zone::write(uint32_t active,const Keyboard::Preset& preset)
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

    const auto data   = report(active,preset);
    const int  result = hid_send_feature_report(device,data.data(),data.size());

    if(result < 0)
    {
        LOG_W(QString("4-zone RGB keyboard: write failed: ").append(QString::fromWCharArray(hid_error(device))));
    }

    hid_close(device);

    return result >= 0;
}

}
