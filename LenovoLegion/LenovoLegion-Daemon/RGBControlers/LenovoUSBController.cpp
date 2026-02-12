// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#include "LenovoUSBController.h"

#include <Core/LoggerHolder.h>

#include "StringUtils.h"

#include <iomanip>
#include <sstream>

#include <string.h>
#include <thread>


namespace LenovoLegionDaemon {


LenovoUSBController::LenovoUSBController(hid_device* dev_handle, const char* path, uint16_t in_pid, uint16_t in_vid) :
    m_dev(dev_handle),
    m_location(path),
    m_pid(in_pid),
    m_vid(in_vid)
{
    /*---------------------------------------------------------*\
    | Get device name from HID manufacturer and product strings |
    \*---------------------------------------------------------*/
    wchar_t string[HID_MAX_STR + 1] = {0};
    if(hid_get_manufacturer_string(m_dev, string, HID_MAX_STR))
    {
        hid_close(m_dev);
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,QString::asprintf("Unable to get device manufacturer string what=(%s)",StringUtils::wstring_to_string(hid_error(m_dev)).c_str()).toStdString().c_str());
    }
    m_name = StringUtils::wstring_to_string(string);

    if(hid_get_product_string(m_dev, string, HID_MAX_STR))
    {
        hid_close(m_dev);
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,QString::asprintf("Unable to get device product string what=(%s)",StringUtils::wstring_to_string(hid_error(m_dev)).c_str()).toStdString().c_str());
    }
    m_name.append(" ").append(StringUtils::wstring_to_string(string));


    if(hid_get_serial_number_string(m_dev, string, HID_MAX_STR))
    {
        hid_close(m_dev);
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,QString::asprintf("Unable to get device serial string what=(%s)",StringUtils::wstring_to_string(hid_error(m_dev)).c_str()).toStdString().c_str());
    }
    m_serial = StringUtils::wstring_to_string(string);


    if(!isCompatible())
    {
        hid_close(m_dev);
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_NOT_COMPATIBLE,"Device is not compatible");
    }

    initilizeKeyMap();
}

LenovoUSBController::~LenovoUSBController()
{
    hid_close(m_dev);
}

uint16_t LenovoUSBController::getPid() const
{
    return m_pid;
}

uint16_t LenovoUSBController::getVid() const
{
    return m_vid;
}

std::string LenovoUSBController::getName() const
{
    return m_name;
}

std::string LenovoUSBController::getLocation() const
{
    return("HID: " + m_location);
}

void LenovoUSBController::setProfileDescription(uint8_t profile_id,const  std::vector<led_group>& led_groups)
{
    LOG_T(QString::asprintf("LenovoUSBController::setProfileDescription: Setting LED groups for profile %d", profile_id));

    ByteArray packet = serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::EffectChange,{.value1 = profile_id,
                                                                                      .value2 = {}
                                                                                      },[&led_groups](){
        ByteArray payload;

        payload.push_back(0x01); //Unknown field
        payload.push_back(0x01); //Unknown field

        for(size_t group = 0; group < led_groups.size(); ++group)
        {
            payload.push_back(static_cast<uint8_t>(group + 1));  //Group index
            payload.push_back(0x06);
            payload.push_back(0x01);
            payload.push_back(led_groups[group].m_mode);
            payload.push_back(0x02);
            payload.push_back(led_groups[group].m_speed);
            payload.push_back(0x03);
            payload.push_back(led_groups[group].m_spin);
            payload.push_back(0x04);
            payload.push_back(led_groups[group].m_direction);
            payload.push_back(0x05);
            payload.push_back(led_groups[group].m_color_mode);
            payload.push_back(0x06);
            payload.push_back(static_cast<char>(0x00));

            payload.push_back(static_cast<uint8_t>(led_groups[group].m_colors.size()));
            for(RGBColor c : led_groups[group].m_colors)
            {
                payload.push_back(RGBGetRValue(c));
                payload.push_back(RGBGetGValue(c));
                payload.push_back(RGBGetBValue(c));
            }

            payload.push_back(static_cast<uint8_t>(led_groups[group].m_leds.size()));
            for(auto led : led_groups[group].m_leds)
            {
                payload.push_back(led      & 0xFF);
                payload.push_back(led >> 8 & 0xFF);
            }
        }
        return payload;
    }(),0x00);


    sendFeatureReport(packet);
}

uint8_t LenovoUSBController::getCurrentProfileId() const
{
    LOG_T("LenovoUSBController::getCurrentProfileId: Getting current profile ID");

    // Retry up to 10 times if we get profile 0 (device may be in reset state)
    for(int attempt = 0; attempt < 10; ++attempt)
    {
        ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::Profile));
        
        if(result.size() >= 5 && result[4] != 0x00)
        {
            if(attempt > 0)
            {
                LOG_T(QString::asprintf("getCurrentProfileId: Got valid profile after %d retries", attempt));
            }
            return result[4];
        }
        
        if(attempt < 9)
        {
            LOG_T("getCurrentProfileId: Got profile 0 (device may be in reset state), retrying...");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    LOG_W("getCurrentProfileId: Device returned profile 0 after all retries");
    return 0;
}

uint8_t LenovoUSBController::getCurrentBrightness() const
{
    LOG_T("LenovoUSBController::getCurrentBrightness: Getting current brightness");

    ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::GetBrightness));
    return  result.size() >= 5 ? result[4] : 0;
}

bool LenovoUSBController::getLogoStatus() const
{
    LOG_T("LenovoUSBController::getLogoStatus: Getting current logo LED status");

    ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::GetLogoStatus));
    return  result.size() >= 5 ? result[4] == 0x01 : false;
}

std::unordered_map<uint16_t, RGBColor> LenovoUSBController::getState() const
{
    LOG_T("LenovoUSBController::getState: Getting current LED state");

    ByteArray result = getFeatureReport();

    std::unordered_map<uint16_t, RGBColor> ledState;

    if(result.size() < 5)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get LED state - invalid response size");
    }

    /*
     * Parse response
     */
    size_t i = 4; // skip header
    while(i + 4 < result.size())
    {
        uint16_t ledId = result[i] | (result[i + 1] << 8);
        RGBColor color = ToRGBColor(result[i + 2], result[i + 3], result[i + 4]);

        ledState[ledId] = color;

        i += 5;
    }

    return ledState;
}

const LenovoUSBController::KeyMap& LenovoUSBController::getKeyMap() const
{
    return m_keyMap;
}

bool LenovoUSBController::isCompatible()
{
    LOG_T("LenovoUSBController::isCompatibility: Getting device compatibility");

    ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::Compatibility));

    return result.size() >= 5 ? result[4] == 0x00 : false;
}

void LenovoUSBController::initilizeKeyMap()
{
    LOG_T("LenovoUSBController::initilizeKeyMap: Initializing key map");

    ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::KeyCount,{0x07,{}}));

    if(result.size() < 7)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid response size");
    }


    m_keyMap.m_height = result[5]; // Indexes
    m_keyMap.m_width  = result[6]; // KeysPerIndex

    m_keyMap.m_keyCodes.resize(m_keyMap.m_width);
    for (auto & keyCodes : m_keyMap.m_keyCodes)
    {
        keyCodes.resize(m_keyMap.m_height,0x0000);
    }

    for (uint8_t y = 0; y < m_keyMap.m_height; y++)
    {
        ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::KeyPage,{ 0x07, y }));

        if(result.size() < 7)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid response size");
        }

        if(result[5] != y)
        {
            THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid page index in response");
        }

        size_t cnt = 6;
        for(uint8_t x = 0; x < m_keyMap.m_width; ++x, cnt += 3)
        {
            if(cnt + 2 >= result.size())
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid response size");
            }

            m_keyMap.m_keyCodes[x][y] = result[cnt + 1] | (result[cnt + 2] << 8);
        }
    }

    {
         ByteArray result = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::KeyPage,{ 0x08, 0 }));

         if(result.size() < 7)
         {
             THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid response size");
         }

         if(result[5] != 0)
         {
             THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid page index in response");
         }

        for(size_t cnt = 6; cnt < result.size(); cnt += 3)
        {
            if(cnt + 2 >= result.size())
            {
                THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to get key map - invalid response size");
            }

            m_keyMap.m_additionalKeyCodes.push_back(result[cnt + 1] | (result[cnt + 2] << 8));
        }
    }
}

LenovoUSBController::ByteArray LenovoUSBController::serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE type, const Params& params,ByteArray payload ,uint8_t size)
{
    ByteArray packet (PACKET_SIZE, 0x00);

    size_t i    = 0;
    packet[i++] = REPORT_ID;
    packet[i++] = type;
    packet[i++] = size % 256;
    packet[i++] = 0x03;

    if(params.value1.has_value())
    {
        packet[i++] = params.value1.value();
    }

    if(params.value2.has_value())
    {
        packet[i++] = params.value2.value();
    }

    if(payload.size() > PACKET_SIZE - i)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DATA_PAYLOAD_TOO_LARGE,"Payload size exceeds maximum packet size");
    }

    std::copy(payload.begin(), payload.end(),packet.begin() + i);


    if(size == 0x00)
    {
        packet[2] = static_cast<uint8_t>((i + payload.size()) % 256);
    }

    return packet;
}

std::string LenovoUSBController::convertBytesArrayToHex(const ByteArray& array)
{
    std::ostringstream stream;

    for(const uint8_t oneInputByte : array)
    {
        stream << (stream.tellp()==0 ? "" : " ") << std::setw(2) << std::setfill('0') << std::hex << (int)oneInputByte;
    }

    return stream.str();
}


std::string LenovoUSBController::getSerialString() const
{
    return m_serial;
}


void LenovoUSBController::setBrightness(uint8_t brightness)
{
    LOG_T(QString::asprintf("LenovoUSBController::setBrightness: Setting brightness to %d", brightness));

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::Brightness, {.value1 = brightness,
                                                                                     .value2 = {}
                                                                                    }));
}

void LenovoUSBController::setProfile(uint8_t profile_id)
{
    LOG_T(QString::asprintf("LenovoUSBController::switchProfileTo: Switching to profile %d", profile_id));

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::ProfileChange, {.value1 = profile_id,
                                                                                        .value2 = {}
                                                                                       }));
 }

void LenovoUSBController::setProfileDefault(uint8_t profile_id)
{
    LOG_T(QString::asprintf("LenovoUSBController::setProfileDefault: Setting profile %d as default", profile_id));

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::ProfileDefault, {.value1 = profile_id,
                                                                                         .value2 = {}
                                                                                         }));
}

void LenovoUSBController::setLogoStatus(bool enabled)
{
    LOG_T(QString::asprintf("LenovoUSBController::setLogoStatus: Setting logo LED status to %s", enabled ? "enabled" : "disabled"));

    sendFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::LogoStatus, {.value1 = enabled ? 0x01 : 0x00,
                                                                                     .value2 = {}
                                                                                     }));
}

std::vector<LenovoUSBController::led_group> LenovoUSBController::getProfileDescription(uint8_t profile_id) const
{
    LOG_T(QString::asprintf("LenovoUSBController::getProfileDescription: Getting profile description for profile %d", profile_id));

    // Retry up to 10 times if we get invalid profile data (device may be in reset state)
    for(int attempt = 0; attempt < 10; ++attempt)
    {
        ByteArray response = sendAndGetFeatureReport(serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE::Effect, {.value1 = profile_id,
                                                                                                                 .value2 = {}
                                                                                                                 }));

        // Check if response is valid and profile ID matches
        if(response.size() < 5 || response[4] == 0x00)
        {
            if(attempt < 9)
            {
                LOG_T(QString::asprintf("getProfileDescription: Got invalid profile data (profile=%d), retrying... (attempt %d/10)", response.size() >= 5 ? response[4] : -1, attempt + 1));
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            continue;
        }

        /*
         * Parse response
         */
        std::vector<led_group> groups;

        /*
         * Set Header
         */
        Header header( response[4]      // profile
                      ,response[1]      // operation
                      ,response[2]);    // size


        LOG_T(QString::asprintf("LenovoUSBController::getProfileDescription: Received profile settings for profile %d, operation: %X, size: %X", header.m_profile, header.m_operation, header.m_size));

        /*
         * Parse Data
         */
        size_t i =  Header::m_dataIndx;
        qsizetype lastEffectNo = 1;
        while(i < response.size())
        {
            int effectNo = response[i++];

            if (effectNo < lastEffectNo)
                break;

            lastEffectNo = effectNo;

            led_group group {0,0,0,0,0,{},{}};

            /*-----------------*\
            |read group settings|
            \*-----------------*/
            if(i >= response.size())
            {
                LOG_E(QString::asprintf("LenovoUSBController::getProfileSettings: Incomplete group settings, packet: %s",convertBytesArrayToHex(response).c_str()));
                break;
            }

            size_t cnt = response[i++];
            if(cnt != 6)
            {
                LOG_E(QString::asprintf("LenovoUSBController::getProfileSettings: Invalid group settings count = %ld, packet: %s", cnt,convertBytesArrayToHex(response).c_str()));
                break;
            }
            for(size_t j = 0; j < cnt && (i + 1) < response.size(); j++, i+=2)
            {
                switch(response[i])
                {
                case 0x01:
                group.m_mode  = response[i+1];
                break;
                case 0x02:
                group.m_speed = response[i+1];
                break;
                case 0x03:
                group.m_spin = response[i+1];
                break;
                case 0x04:
                group.m_direction = response[i+1];
                break;
                case 0x05:
                group.m_color_mode = response[i+1];
                break;
                case 0x06:
                //group.mode = response[i+1];
                break;
                }
            }

            /*-----------------*\
            |read group colors  |
            \*-----------------*/
            if(i >= response.size())
            {
                LOG_E(QString::asprintf("LenovoUSBController::getProfileSettings: Incomplete group colors, packet: %s",convertBytesArrayToHex(response).c_str()));
                break;
            }

            cnt = response[i++];
            for(size_t j = 0; j < cnt && (i + 2) < response.size(); j++, i+=3)
            {
                group.m_colors.push_back(ToRGBColor(response[i],response[i+1],response[i+2]));
            }

            /*-----------------*\
            |read group LEDs    |
            \*-----------------*/
            if(i >= response.size())
            {
                LOG_E(QString::asprintf("LenovoUSBController::getProfileSettings: Incomplete group LEDs, packet: %s",convertBytesArrayToHex(response).c_str()));
                break;
            }

            cnt = response[i++];
            for(size_t j = 0; j < cnt && (i + 1) < response.size(); j++, i+=2)
            {
                group.m_leds.push_back(response[i] | response[i+1] << 8);
            }

            groups.push_back(group);
        }

        return groups;
    }

    LOG_W("getProfileDescription: Device returned invalid data after all retries");

    return std::vector<LenovoUSBController::led_group>();
}

void LenovoUSBController::sendFeatureReport(const ByteArray& packet) const
{
    LOG_T(QString::asprintf("LenovoUSBController::sendFeatureReport: Sending packet: %s", convertBytesArrayToHex(packet).c_str()));

    int ret = hid_send_feature_report(m_dev, packet.data(), packet.size());
    if(ret < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,QString::asprintf("Unable to send feature report to device what=(%s)", StringUtils::wstring_to_string(hid_error(m_dev)).c_str()).toStdString().c_str());
    }

    if(static_cast<size_t>(ret) != packet.size())
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,"Unable to send complete feature report to device");
    }
}


LenovoUSBController::ByteArray LenovoUSBController::sendAndGetFeatureReport(const ByteArray &packet) const
{
    sendFeatureReport(packet);
    return getFeatureReport();
}


LenovoUSBController::ByteArray LenovoUSBController::getFeatureReport() const
{
    ByteArray response(PACKET_SIZE, REPORT_ID);

    int ret = hid_get_feature_report(m_dev, response.data(), response.size());
    if(ret < 0)
    {
        THROW_EXCEPTION(exception_T,ERROR_CODES::DEVICE_COMMUNICATION_ERROR,QString::asprintf("Unable to get feature report from device what=(%s)", StringUtils::wstring_to_string(hid_error(m_dev)).c_str()).toStdString().c_str());
    }

    response.resize(ret);

    LOG_T(QString::asprintf("LenovoUSBController::getFeatureReport: Getting size=%d, packet: %s",ret, convertBytesArrayToHex(response).c_str()));

    return response;
}

}
