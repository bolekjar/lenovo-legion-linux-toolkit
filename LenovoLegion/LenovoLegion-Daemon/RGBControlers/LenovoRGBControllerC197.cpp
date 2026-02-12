#include "LenovoRGBControllerC197.h"

#include "RGBControllerKeyNames.h"

#include <Core/LoggerHolder.h>

namespace LenovoLegionDaemon {

LenovoRGBControllerC197::LenovoRGBControllerC197(LenovoUSBController *controller_ptr) :
    LenovoRGBController(  controller_ptr,
                        {.min = 1,.max = 6,.active = 1},                            /* profiles */
                        {.min = 0,.max = 9,.active = 0},                            /* brightness levels */
                        "Lenovo RGB Controller",            /* name */
                        "Lenovo",                           /* vendor */
                        "Lenovo RGB Controller",            /* description */
                        controller_ptr->getSerialString(),  /* serial */
                        controller_ptr->getLocation(),      /* location */
                        DEVICE_TYPE_KEYBOARD,
                        {                                   /* modes */
                            {
                             "Screw Rainbow",
                             MODE_SCREW_RAINBOW,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_SPINLR | MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             0,
                             0,
                             (SPEED_MAX + 1)  / 2,
                             MODE_DIRECTION_RIGHT,
                             MODE_COLORS_NONE,
                             {},
                             },
                            {
                             "Rainbow Wave",
                             MODE_RAINBOW_WAVE,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             0,
                             0,
                             (SPEED_MAX + 1) / 2,
                             MODE_DIRECTION_RIGHT,
                             MODE_COLORS_NONE,
                             {},
                             },
                            {
                             "Color Change",
                             MODE_COLOR_CHANGE,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                            {
                             "Color Pulse",
                             MODE_COLOR_PULSE,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                            {
                             "Color Wave",
                             MODE_COLOR_WAVE,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_DIRECTION_LR | MODE_FLAG_HAS_DIRECTION_UD | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             MODE_DIRECTION_RIGHT,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                            {
                             "Smooth",
                             MODE_SMOOTH,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                            {
                             "Rain",
                             MODE_RAIN,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_PER_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                            {
                             "Ripple",
                             MODE_RIPPLE,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_ALL_KB_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                             {
                               "Audio Bounce Lighting (Not working)", // Not working in Linux due to missing implementation in sound driver, can be selected but no effect - is supported by Windows Lenovo Vantage and RGB Controler then must be present otherwise error is shown
                               MODE_AUDIO_BOUNCE_LIGHTING,
                               MODE_FLAG_HAS_ALL_LED_SELECTION,
                               0,
                               0,
                               0,
                               0,
                               0,
                               MODE_COLORS_NONE,
                               {},
                               {}
                             },
                             {
                               "Audio Ripple Lighting (Not working)", // Not working in Linux due to missing implementation in sound driver, can be selected but no effect - is supported by Windows Lenovo Vantage and RGB Controler then must be present otherwise error is shown
                               MODE_AUDIO_RIPPLE_LIGHTING,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               MODE_COLORS_NONE,
                               {},
                               {}
                             },
                            {
                             "Always",
                             MODE_ALWAYS,
                             MODE_FLAG_HAS_PER_LED_SELECTION | MODE_FLAG_HAS_PER_LED_COLOR |  MODE_FLAG_HAS_AUTOMATIC_SAVE,
                             0,
                             0,
                             0,
                             0,
                             0,
                             0,
                             MODE_COLORS_PER_LED,
                             {},
                             },
                            {
                             "Type Lighting ",
                             MODE_TYPE_LIGHTING,
                             MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_RANDOM_COLOR | MODE_FLAG_HAS_AUTOMATIC_SAVE | MODE_FLAG_HAS_ALL_KB_LED_SELECTION,
                             SPEED_MIN,
                             SPEED_MAX,
                             1,
                             4,
                             (SPEED_MAX + 1) / 2,
                             0,
                             MODE_COLORS_RANDOM,
                             {0xFFF500, 0xFFF500, 0xFFF500, 0xFFF500},
                             },
                             {
                            "Aura (Not working)", // Not working need implementation
                            MODE_LEGION_AURASYNC,
                            MODE_FLAG_HAS_DIRECT_CONTROL | MODE_FLAG_HAS_ALL_LED_SELECTION,
                            0,
                            0,
                            0,
                            0,
                            0,
                            0,
                            MODE_COLORS_NONE,
                            {},
                             }
                        },
                        {       /* zones */
                            {
                                "Keyboard",
                                ZONE_TYPE_MATRIX,
                                0,
                                [controller_ptr](){
                                    unsigned int leds_count = 0;

                                    for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                    {
                                        for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                        {
                                            if(controller_ptr->getKeyMap().m_keyCodes[x][y] != 0x0000)
                                            {
                                                leds_count++;
                                            }
                                        }
                                    }
                                    return leds_count;
                                }()
                                ,
                                [controller_ptr](){
                                    unsigned int leds_count = 0;

                                    for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                    {
                                        for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                        {
                                            if(controller_ptr->getKeyMap().m_keyCodes[x][y] != 0x0000)
                                            {
                                                leds_count++;
                                            }
                                        }
                                    }
                                    return leds_count;
                                }()
                                ,
                                [controller_ptr](){
                                    unsigned int leds_count = 0;

                                    for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                    {
                                        for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                        {
                                            if(controller_ptr->getKeyMap().m_keyCodes[x][y] != 0x0000)
                                            {
                                                leds_count++;
                                            }
                                        }
                                    }
                                    return leds_count;
                                }()
                                ,
                                {
                                    [controller_ptr](){
                                        std::vector<led> leds;

                                        for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                        {
                                            for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                            {
                                                if(controller_ptr->getKeyMap().m_keyCodes[x][y] != 0x0000 )
                                                {
                                                    if(KeyCodesToName.count(controller_ptr->getKeyMap().m_keyCodes[x][y]) > 0)
                                                    {
                                                        leds.push_back(led {KeyCodesToName.at(controller_ptr->getKeyMap().m_keyCodes[x][y]), controller_ptr->getKeyMap().m_keyCodes[x][y]});
                                                    }
                                                    else
                                                    {
                                                        leds.push_back(led {QString::number(controller_ptr->getKeyMap().m_keyCodes[x][y]).toStdString() ,
                                                                           controller_ptr->getKeyMap().m_keyCodes[x][y]});
                                                    }
                                                }
                                            }
                                        }
                                        return leds;
                                    }()
                                },
                                {
                                    [controller_ptr](){
                                        matrix_map_type matrix_map;

                                        matrix_map.height = controller_ptr->getKeyMap().m_height;
                                        matrix_map.width  = controller_ptr->getKeyMap().m_width;

                                        int i = 0;
                                        for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                        {
                                            for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                            {
                                                if(controller_ptr->getKeyMap().m_keyCodes[x][y] == 0x0000)
                                                {
                                                    matrix_map.map.push_back(NA);
                                                }
                                                else
                                                {
                                                    matrix_map.map.push_back(i);
                                                    i++;
                                                }
                                            }
                                        }
                                        return matrix_map;
                                    }()
                                },
                                0 /* start_idx */,
                            },
                           {
                            "Logo",
                            ZONE_TYPE_LINEAR,
                            0,
                               [controller_ptr](){
                                   unsigned int leds_count = 0;

                                   for(auto led : controller_ptr->getKeyMap().m_additionalKeyCodes)
                                   {
                                       if(led != 0x0000)
                                       {
                                           leds_count++;
                                       }
                                   }
                                   return leds_count;
                               }(),
                               [controller_ptr](){
                                   unsigned int leds_count = 0;

                                   for(auto led : controller_ptr->getKeyMap().m_additionalKeyCodes)
                                   {
                                       if(led != 0x0000)
                                       {
                                           leds_count++;
                                       }
                                   }
                                   return leds_count;
                               }(),
                               [controller_ptr](){
                                   unsigned int leds_count = 0;

                                   for(auto led : controller_ptr->getKeyMap().m_additionalKeyCodes)
                                   {
                                       if(led != 0x0000)
                                       {
                                           leds_count++;
                                       }
                                   }
                                   return leds_count;
                               }(),
                            {
                               [controller_ptr](){
                                   std::vector<led> leds;

                                   for(auto led : controller_ptr->getKeyMap().m_additionalKeyCodes)
                                   {
                                       if(led != 0x0000)
                                       {
                                           leds.push_back({QString::number(led).toStdString(), led});
                                       }
                                   }
                                   return leds;
                               }(),
                            },
                            {
                             0,
                             0,
                             {}
                            },
                            [controller_ptr](){
                                unsigned int leds_count = 0;

                                for (int y = 0; y < controller_ptr->getKeyMap().m_height; y++)
                                {
                                    for (int x = 0; x < controller_ptr->getKeyMap().m_width; x++)
                                    {
                                        if(controller_ptr->getKeyMap().m_keyCodes[x][y] != 0x0000)
                                        {
                                            leds_count++;
                                        }
                                    }
                                }
                                return leds_count;
                            }()
                           }
                        },
                        20 /* max effects */,
                        controller_ptr->getKeyMap().m_additionalKeyCodes.size() > 0 /* has logo control */
                        )
{
    /*
     * Read active profile settings
     */

    DeviceRefresh();
}

}
