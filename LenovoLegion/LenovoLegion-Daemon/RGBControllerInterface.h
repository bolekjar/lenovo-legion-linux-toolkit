// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#pragma once

#include <QString>
#include <QObject>

#include <string>
#include <vector>
#include <set>

namespace LenovoLegionDaemon {

/*------------------------------------------------------------------*\
| RGB Color Type and Conversion Macros                               |
\*------------------------------------------------------------------*/
typedef unsigned int RGBColor;


#define RGBGetRValue(rgb)   (rgb & 0x000000FF)
#define RGBGetGValue(rgb)   ((rgb >> 8) & 0x000000FF)
#define RGBGetBValue(rgb)   ((rgb >> 16) & 0x000000FF)

#define ToRGBColor(r, g, b) ((LenovoLegionDaemon::RGBColor)((b << 16) | (g << 8) | (r)))


/*------------------------------------------------------------------*\
| Mode Flags                                                         |
\*------------------------------------------------------------------*/
enum ModeFlags : unsigned int
{
    MODE_FLAG_HAS_SPEED                 = (1 << 0), /* Mode has speed parameter         */
    MODE_FLAG_HAS_DIRECTION_LR          = (1 << 1), /* Mode has left/right parameter    */
    MODE_FLAG_HAS_DIRECTION_UD          = (1 << 2), /* Mode has up/down parameter       */
    MODE_FLAG_HAS_DIRECTION_SPINLR      = (1 << 3), /* Mode has spin left/right arameter*/
    MODE_FLAG_HAS_PER_LED_COLOR         = (1 << 4), /* Mode has per-LED colors          */
    MODE_FLAG_HAS_MODE_SPECIFIC_COLOR   = (1 << 5), /* Mode has mode specific colors    */
    MODE_FLAG_HAS_RANDOM_COLOR          = (1 << 6), /* Mode has random color option     */
    MODE_FLAG_HAS_AUTOMATIC_SAVE        = (1 << 7), /* Mode automatically saves         */
    MODE_FLAG_HAS_DIRECT_CONTROL        = (1 << 8), /* Mode supports Direct Control         */
    MODE_FLAG_HAS_PER_LED_SELECTION     = (1 << 9), /* Mode has per led selection option    */
    MODE_FLAG_HAS_ALL_LED_SELECTION     = (1 << 10),/* Mode has all led selection option    */
    MODE_FLAG_HAS_ALL_KB_LED_SELECTION  = (1 << 11),/* Mode has all keyboard led selection option */
};




/*------------------------------------------------------------------*\
| Mode Directions                                                    |
\*------------------------------------------------------------------*/
enum ModeDirection : unsigned int
{
    MODE_DIRECTION_LEFT         = 0,        /* Mode direction left              */
    MODE_DIRECTION_RIGHT        = 1,        /* Mode direction right             */
    MODE_DIRECTION_UP           = 2,        /* Mode direction up                */
    MODE_DIRECTION_DOWN         = 3,        /* Mode direction down              */
    MODE_DIRECTION_SPINLEFT     = 4,        /* Mode direction spin left         */
    MODE_DIRECTION_SPINRIGHT    = 5,        /* Mode direction spin right        */
    MODE_DIRECTION_NA           = 6         /* Mode has no direction            */
};
inline unsigned int modeDirectionToValue(ModeDirection direction){ return static_cast<unsigned int>(direction);}
inline ModeDirection valueToModeDirection(unsigned int value){ return static_cast<ModeDirection>(value);}
inline QString modeDirectionToString(ModeDirection direction)
{
    switch(direction)
    {
        case MODE_DIRECTION_LEFT:        return QObject::tr("Left");
        case MODE_DIRECTION_RIGHT:       return QObject::tr("Right");
        case MODE_DIRECTION_UP:          return QObject::tr("Up");
        case MODE_DIRECTION_DOWN:        return QObject::tr("Down");
        case MODE_DIRECTION_SPINLEFT:    return QObject::tr("SpinLeft");
        case MODE_DIRECTION_SPINRIGHT:    return QObject::tr("SpinRight");
        case MODE_DIRECTION_NA:          return QObject::tr("N/A");
        default:                         return QObject::tr("Unknown");
    }
}
inline ModeDirection stringToModeDirection(const QString& str)
{
    if(str == QObject::tr("Left"))           return MODE_DIRECTION_LEFT;
    else if(str == QObject::tr("Right"))     return MODE_DIRECTION_RIGHT;
    else if(str == QObject::tr("Up"))        return MODE_DIRECTION_UP;
    else if(str == QObject::tr("Down"))      return MODE_DIRECTION_DOWN;
    else if(str == QObject::tr("SpinLeft"))  return MODE_DIRECTION_SPINLEFT;
    else if(str == QObject::tr("SpinRight")) return MODE_DIRECTION_SPINRIGHT;
    else                                     return MODE_DIRECTION_NA;
}




/*------------------------------------------------------------------*\
| Mode Color Types                                                   |
\*------------------------------------------------------------------*/
enum ModeColorType : unsigned int
{
    MODE_COLORS_NONE            = 0,        /* Mode has no colors               */
    MODE_COLORS_PER_LED         = 1,        /* Mode has per LED colors selected */
    MODE_COLORS_MODE_SPECIFIC   = 2,        /* Mode specific colors selected    */
    MODE_COLORS_RANDOM          = 3,        /* Mode has random colors selected  */
    MODE_COLORS_MAX             = 4         /* Maximum number of color types    */
};

inline unsigned int modeColorTypeToValue(ModeColorType type){ return static_cast<unsigned int>(type);}
inline ModeColorType valueToModeColorType(unsigned int value){ return static_cast<ModeColorType>(value);}
inline QString modeColorTypeToString(ModeColorType type)
{
    switch(type)
    {
        case MODE_COLORS_NONE:            return QObject::tr("None");
        case MODE_COLORS_PER_LED:         return QObject::tr("Per LED");
        case MODE_COLORS_MODE_SPECIFIC:   return QObject::tr("Mode Specific");
        case MODE_COLORS_RANDOM:          return QObject::tr("Random");
        default:                          return QObject::tr("Unknown");
    }
}
inline ModeColorType stringToModeColorType(const QString& str)
{
    if(str == QObject::tr("None"))               return MODE_COLORS_NONE;
    else if(str == QObject::tr("Per LED"))       return MODE_COLORS_PER_LED;
    else if(str == QObject::tr("Mode Specific")) return MODE_COLORS_MODE_SPECIFIC;
    else if(str == QObject::tr("Random"))        return MODE_COLORS_RANDOM;
    else                                        return MODE_COLORS_NONE; // Default
}


/*------------------------------------------------------------------*\
| Mode Class                                                         |
\*------------------------------------------------------------------*/
struct mode
{
    /*--------------------------------------------------------------*\
    | Mode Information                                               |
    \*--------------------------------------------------------------*/
    const std::string         name;             /* Mode name                    */
    const int                 value         = 0;/* Device-specific mode value   */
    const unsigned int        flags         = 0;/* Mode flags bitfield          */
    const unsigned int        speed_min     = 0;/* speed minimum value          */
    const unsigned int        speed_max     = 0;/* speed maximum value          */
    const unsigned int        colors_min    = 0;/* minimum number of mode colors*/
    const unsigned int        colors_max    = 0;/* maximum numver of mode colors*/

    /*--------------------------------------------------------------*\
    | Mode Settings                                                  |
    \*--------------------------------------------------------------*/
    const unsigned int        speed         =0; /* Mode speed parameter value   */
    const unsigned int        direction     =0; /* Mode direction value         */
    const unsigned int        color_mode    =0; /* Mode color selection         */

    std::vector<RGBColor>     colors;           /* mode-specific colors         */
};

/*------------------------------------------------------------------*\
| LED Struct                                                         |
\*------------------------------------------------------------------*/
typedef struct
{
    std::string         name;   /* LED name                     */
    unsigned int        value;  /* Device-specific LED value    */
} led;


/*------------------------------------------------------------------*\
| Zone Types                                                         |
\*------------------------------------------------------------------*/
typedef int zone_type;

enum
{
    ZONE_TYPE_SINGLE,
    ZONE_TYPE_LINEAR,
    ZONE_TYPE_MATRIX
};

/*------------------------------------------------------------------*\
| Matrix Map Struct                                                  |
\*------------------------------------------------------------------*/
typedef struct
{
    unsigned int                 height;
    unsigned int                 width;
    std::vector<unsigned int>    map;
} matrix_map_type;

/*------------------------------------------------------------------*\
| Zone Class                                                         |
\*------------------------------------------------------------------*/
struct zone
{
    const std::string                       name;           /* Zone name                */
    const zone_type                         type;           /* Zone type                */
    const unsigned int                      flags;          /* Zone flags bitfield      */
    const unsigned int                      leds_count;     /* Number of LEDs in zone   */
    const unsigned int                      leds_min;       /* Minimum number of LEDs   */
    const unsigned int                      leds_max;       /* Maximum number of LEDs   */
    const std::vector<led>                  leds;           /* List of LEDs in zone     */
    const matrix_map_type                   matrix_map;     /* Matrix map               */
    const unsigned int                      start_idx;      /* Start index in LED array */
};

/*
 * Led Group Effect Struct
 */
struct led_group_effect
{
    int                     m_mode;

    unsigned int            m_speed;
    unsigned int            m_direction;
    unsigned int            m_color_mode;

    std::vector<RGBColor>   m_colors;
    std::vector<led>        m_leds;
};


struct Profiles
{
    unsigned int min;
    unsigned int max;
    unsigned int active;
};

struct Brightnesses
{
    unsigned int min;
    unsigned int max;
    unsigned int active;
};

/*------------------------------------------------------------------*\
| Device Types                                                       |
|   The enum order should be maintained as is for the API however    |
|   DEVICE_TYPE_UNKNOWN needs to remain last. Any new device types   |
|   need to be inserted at the end of the list but before unknown.   |
\*------------------------------------------------------------------*/
typedef int device_type;

enum
{
    DEVICE_TYPE_KEYBOARD,
    DEVICE_TYPE_UNKNOWN
};

/*------------------------------------------------------------------*\
| RGBController Callback Types                                       |
\*------------------------------------------------------------------*/
typedef void (*RGBControllerCallback)(void *);

std::string device_type_to_str(device_type type);

class RGBControllerInterface
{
public:
    virtual ~RGBControllerInterface() {}

    /*
     * Getters
     */
    virtual device_type                   GetDeviceType()      const                                            = 0;

    /*
     * Profiles Information
     */
    virtual const Profiles&               GetProfiles()        const                                            = 0;
    virtual void                          SetProfile(unsigned int profileIdx)                                   = 0;

    /*
     * Zone Information
     */
    virtual const std::vector<zone>&      GetZones()                          const                             = 0;
    virtual std::string                   GetZoneName(unsigned int zone)      const                             = 0;
    virtual unsigned int                  GetLEDsInZone(unsigned int zone)    const                             = 0;

    /*
     * Britness Information
     */
    virtual const Brightnesses&           GetBrightness()    const                                               = 0;
    virtual void                          SetBrightness(unsigned int brightness)                                 = 0;


    /*
     * Effects Information
     */
    virtual const
        std::vector<led_group_effect>&    GetEffects()              const                                        = 0;
    virtual unsigned int                  GetMaxEffects()           const                                        = 0;
    virtual const led_group_effect&       GetEffect(unsigned int effectIdx)    const                             = 0;
    virtual void                          SetEfects(const std::vector<led_group_effect>& effects)                = 0;
    virtual void                          AddEffect(const led_group_effect& effect)                              = 0;
    virtual void                          RemoveEffect(unsigned int effectIdx)                                   = 0;
    virtual void                          ClearEffects()                                                         = 0;
    virtual void                          ResetEffectsToDefault()                                                = 0;



    /*
     * Mode Information
     */
    virtual const std::vector<mode>&      GetModes()                          const                             = 0;
    virtual std::string                   GetModeNameByIdx(unsigned int idx)  const                             = 0;
    virtual mode                          GetModeByIdx(unsigned int idx)      const                             = 0;
    virtual mode                          GetModeByModeValue(int mode)        const                             = 0;


    /*
     * Led  maping Information
     */
    virtual const std::vector<led>&       GetLEDs() const                                                       = 0;
    virtual std::string                   GetLEDName(unsigned int led)        const                             = 0;
    virtual std::set<int>                 GetLedsIndexesByDeviceSpecificValue(unsigned int value)   const       = 0;


    /*
     * Get Current colors for all LEDs
     */
    virtual std::vector<RGBColor>         GetStateForAllLeds()    const                                         = 0;


    /*
     * Logo Information
     */
     virtual bool                          HasLogo() const                                                      = 0;
     virtual bool                          GetLogoState() const                                                 = 0;
     virtual void                          SetLogoState(bool on)                                                = 0;

};

}
