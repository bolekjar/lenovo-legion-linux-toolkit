// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include "RGBControllerInterface.h"

#include <Core/ExceptionBuilder.h>

#include <string>
#include <vector>
#include <unordered_map>

#include <hidapi.h>
#include <stdint.h>

namespace LenovoLegionDaemon {

#ifndef HID_MAX_STR
#define HID_MAX_STR                255
#endif

#define PACKET_SIZE                960
#define REPORT_ID                  0x07

class LenovoUSBController
{

protected:
    \
    enum LENOVO_SPECTRUM_OPERATION_TYPE : uint8_t
    {
        Compatibility       = 0xD1,

        KeyCount            = 0xC4,
        KeyPage             = 0xC5,

        ProfileChange       = 0xC8,
        ProfileDefault      = 0xC9,
        Profile             = 0xCA,

        EffectChange        = 0xCB,
        Effect              = 0xCC,

        GetBrightness       = 0xCD,
        Brightness          = 0xCE,

        AuroraStartStop     = 0xD0,
        AuroraSendBitmap    = 0xA1,

        GetLogoStatus       = 0xA5,
        LogoStatus          = 0xA6
    };


    struct Header
    {

        Header(uint8_t m_profile,uint8_t operation, uint8_t size) :
            m_profile(m_profile) ,
            m_operation(static_cast<LENOVO_SPECTRUM_OPERATION_TYPE>(operation)),
            m_size(size % 256)
        {}

        static const uint8_t                 m_dataIndx = 7;

        const uint8_t                        m_profile;
        const LENOVO_SPECTRUM_OPERATION_TYPE m_operation;
        const uint8_t                        m_size;
    };

    struct Params
    {
        const std::optional<uint8_t> value1;
        const std::optional<uint8_t> value2;
    };


    struct KeyMap
    {
        uint8_t m_width;
        uint8_t m_height;

        std::vector<std::vector<uint16_t>> m_keyCodes;
        std::vector<uint16_t>              m_additionalKeyCodes;
    };


    using ByteArray = std::vector<uint8_t>;


public:

    DEFINE_EXCEPTION(LenovoGen9USBController);

    enum ERROR_CODES : int
    {
        DATA_PAYLOAD_TOO_LARGE        = 1,
        DEVICE_COMMUNICATION_ERROR    = 2,
        DEVICE_NOT_COMPATIBLE         = 3
    };

public:

    struct led_group
    {
        uint8_t               m_mode;
        uint8_t               m_speed;
        uint8_t               m_spin;
        uint8_t               m_direction;
        uint8_t               m_color_mode;

        std::vector<RGBColor> m_colors;
        std::vector<uint16_t> m_leds;
    };

public:
    /*--------------*\
    |ctor(s) and dtor|
    \*--------------*/
    LenovoUSBController(hid_device* dev_handle, const char* path, uint16_t in_pid,uint16_t in_vid);
    virtual ~LenovoUSBController();


    /*--------------*\
    |Setters         |
    \*--------------*/
    void setProfileDescription(uint8_t profile_id,const  std::vector<led_group>& led_groups);

    void setBrightness(uint8_t brightness);

    void setProfile(uint8_t profile_id);

    void setProfileDefault(uint8_t profile_id);

    void setLogoStatus(bool enabled);

    /*--------------*\
    |Getters         |
    \*--------------*/
    uint16_t                                getPid()                const;
    uint16_t                                getVid()                const;
    std::string                             getName()               const;
    std::string                             getLocation()           const;
    std::string                             getSerialString()       const;

    uint8_t                                 getCurrentProfileId()   const;
    uint8_t                                 getCurrentBrightness()  const;
    const KeyMap&                           getKeyMap()             const;
    bool                                    getLogoStatus()         const;

    /*
     * Current LED state
     */
    std::unordered_map<uint16_t,RGBColor>   getState()              const;
\
    /*
     * GEt profile description
     */
    std::vector<led_group>  getProfileDescription(uint8_t profile_id) const;

protected:

    static ByteArray    serializeToBuffer(LENOVO_SPECTRUM_OPERATION_TYPE type, const Params& params = {},ByteArray payload = ByteArray(),uint8_t size = 0xC0);
    static std::string  convertBytesArrayToHex(const ByteArray &array);

private:

    bool                    isCompatible();
    void                    initilizeKeyMap();

private:

    /*--------------*\
    |data members    |
    \*--------------*/
    std::string        m_name;
    hid_device * const m_dev;

    const std::string  m_location;
    const uint16_t     m_pid;
    const uint16_t     m_vid;

    std::string        m_version;
    std::string        m_serial;

    KeyMap             m_keyMap;

    /*--------------*\
    |device functions|
    \*--------------*/
protected:
    void        sendFeatureReport(const ByteArray& packet)       const;
    ByteArray   sendAndGetFeatureReport(const ByteArray& packet) const;

    ByteArray   getFeatureReport() const;
};

}
