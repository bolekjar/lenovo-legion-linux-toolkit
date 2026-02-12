// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "DeviceView.h"
#include "Core/LoggerHolder.h"
#include "ProtocolProcessorBase.h"
#include "RGBControllerKeyNames.h"

#include <QPainter>
#include <QResizeEvent>
#include <QStyleOption>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QMouseEvent>

#define MAX_COLS    20
#define PAD_LED     0.1f
#define PAD_TEXT    0.1f
#define PAD_ZONE    1.0f
#define PAD_SEGMENT 0.9f
#define SIZE_TEXT   0.5f

namespace LenovoLegionGui {



DeviceView::DeviceView(QWidget *parent) :
    QWidget(parent),
    initSize(128,128),
    mouseDown(false),
    m_timerId(-1)
{
    controller = NULL;
    numerical_labels = false;
    per_led = false;
    setMouseTracking(1);

    size = width();

    m_timerId = startTimer(50);
    m_keyboardBackgroundImage = "keyboard-background.png";
}

DeviceView::~DeviceView()
{

}

struct led_label
{
    QString label_text;
    QString label_utf8;
};

static const std::map<std::string, led_label> led_label_lookup =
    {
        { LenovoLegionDaemon::KEY_EN_A,                 { "A"     , "A",                }},
        { LenovoLegionDaemon::KEY_EN_B,                 { "B"     , "B",                }},
        { LenovoLegionDaemon::KEY_EN_C,                 { "C"     , "C",                }},
        { LenovoLegionDaemon::KEY_EN_D,                 { "D"     , "D",                }},
        { LenovoLegionDaemon::KEY_EN_E,                 { "E"     , "E",                }},
        { LenovoLegionDaemon::KEY_EN_F,                 { "F"     , "F",                }},
        { LenovoLegionDaemon::KEY_EN_G,                 { "G"     , "G",                }},
        { LenovoLegionDaemon::KEY_EN_H,                 { "H"     , "H",                }},
        { LenovoLegionDaemon::KEY_EN_I,                 { "I"     , "I",                }},
        { LenovoLegionDaemon::KEY_EN_J,                 { "J"     , "J",                }},
        { LenovoLegionDaemon::KEY_EN_K,                 { "K"     , "K",                }},
        { LenovoLegionDaemon::KEY_EN_L,                 { "L"     , "L",                }},
        { LenovoLegionDaemon::KEY_EN_M,                 { "M"     , "M",                }},
        { LenovoLegionDaemon::KEY_EN_N,                 { "N"     , "N",                }},
        { LenovoLegionDaemon::KEY_EN_O,                 { "O"     , "O",                }},
        { LenovoLegionDaemon::KEY_EN_P,                 { "P"     , "P",                }},
        { LenovoLegionDaemon::KEY_EN_Q,                 { "Q"     , "Q",                }},
        { LenovoLegionDaemon::KEY_EN_R,                 { "R"     , "R",                }},
        { LenovoLegionDaemon::KEY_EN_S,                 { "S"     , "S",                }},
        { LenovoLegionDaemon::KEY_EN_T,                 { "T"     , "T",                }},
        { LenovoLegionDaemon::KEY_EN_U,                 { "U"     , "U",                }},
        { LenovoLegionDaemon::KEY_EN_V,                 { "V"     , "V",                }},
        { LenovoLegionDaemon::KEY_EN_W,                 { "W"     , "W",                }},
        { LenovoLegionDaemon::KEY_EN_X,                 { "X"     , "X",                }},
        { LenovoLegionDaemon::KEY_EN_Y,                 { "Y"     , "Y",                }},
        { LenovoLegionDaemon::KEY_EN_Z,                 { "Z"     , "Z",                }},
        { LenovoLegionDaemon::KEY_EN_0,                 { "0"     , "0",                }},
        { LenovoLegionDaemon::KEY_EN_1,                 { "1"     , "1",                }},
        { LenovoLegionDaemon::KEY_EN_2,                 { "2"     , "2",                }},
        { LenovoLegionDaemon::KEY_EN_3,                 { "3"     , "3",                }},
        { LenovoLegionDaemon::KEY_EN_4,                 { "4"     , "4",                }},
        { LenovoLegionDaemon::KEY_EN_5,                 { "5"     , "5",                }},
        { LenovoLegionDaemon::KEY_EN_6,                 { "6"     , "6",                }},
        { LenovoLegionDaemon::KEY_EN_7,                 { "7"     , "7",                }},
        { LenovoLegionDaemon::KEY_EN_8,                 { "8"     , "8",                }},
        { LenovoLegionDaemon::KEY_EN_9,                 { "9"     , "9",                }},
        { LenovoLegionDaemon::KEY_EN_F1,                { "F1"    , "F1",               }},
        { LenovoLegionDaemon::KEY_EN_F2,                { "F2"    , "F2",               }},
        { LenovoLegionDaemon::KEY_EN_F3,                { "F3"    , "F3",               }},
        { LenovoLegionDaemon::KEY_EN_F4,                { "F4"    , "F4",               }},
        { LenovoLegionDaemon::KEY_EN_F5,                { "F5"    , "F5",               }},
        { LenovoLegionDaemon::KEY_EN_F6,                { "F6"    , "F6",               }},
        { LenovoLegionDaemon::KEY_EN_F7,                { "F7"    , "F7",               }},
        { LenovoLegionDaemon::KEY_EN_F8,                { "F8"    , "F8",               }},
        { LenovoLegionDaemon::KEY_EN_F9,                { "F9"    , "F9",               }},
        { LenovoLegionDaemon::KEY_EN_F10,               { "F10"   , "F10",              }},
        { LenovoLegionDaemon::KEY_EN_F11,               { "F11"   , "F11",              }},
        { LenovoLegionDaemon::KEY_EN_F12,               { "F12"   , "F12",              }},
        { LenovoLegionDaemon::KEY_EN_BACK_TICK,         { "`"     , "`",                }},
        { LenovoLegionDaemon::KEY_EN_MINUS,             { "-"     , "-",                }},
        { LenovoLegionDaemon::KEY_EN_PLUS,              { "+"     , "+",                }},
        { LenovoLegionDaemon::KEY_EN_EQUALS,            { "="     , "=",                }},
        { LenovoLegionDaemon::KEY_EN_LEFT_BRACKET,      { "["     , "[",                }},
        { LenovoLegionDaemon::KEY_EN_RIGHT_BRACKET,     { "]"     , "]",                }},
        { LenovoLegionDaemon::KEY_EN_BACK_SLASH,        { "\\"    , "\\",               }},
        { LenovoLegionDaemon::KEY_EN_ANSI_BACK_SLASH,   { "\\"    , "\\",               }},
        { LenovoLegionDaemon::KEY_EN_ISO_BACK_SLASH,    { "\\"    , "\\",               }},
        { LenovoLegionDaemon::KEY_EN_SEMICOLON,         { ";"     , ";",                }},
        { LenovoLegionDaemon::KEY_EN_QUOTE,             { "'"     , "'",                }},
        { LenovoLegionDaemon::KEY_EN_POUND,             { "#"     , "#",                }},
        { LenovoLegionDaemon::KEY_EN_COMMA,             { ","     , ",",                }},
        { LenovoLegionDaemon::KEY_EN_PERIOD,            { "."     , ".",                }},
        { LenovoLegionDaemon::KEY_EN_FORWARD_SLASH,     { "/"     , "/",                }},
        { LenovoLegionDaemon::KEY_EN_ESCAPE,            { "Esc"   , "Esc",              }},
        { LenovoLegionDaemon::KEY_EN_PRINT_SCREEN,      { "Prt"   , "Prt",              }},
        { LenovoLegionDaemon::KEY_EN_SCROLL_LOCK,       { "Scr"   , "Scr",              }},
        { LenovoLegionDaemon::KEY_EN_PAUSE_BREAK,       { "Brk"   , "Brk",              }},
        { LenovoLegionDaemon::KEY_EN_BACKSPACE,         { "Bks"   , "\xE2\x8C\xAB"      }}, // ⌫
        { LenovoLegionDaemon::KEY_EN_INSERT,            { "Ins"   , "Ins",              }},
        { LenovoLegionDaemon::KEY_EN_HOME,              { "Hom"   , "Hom",              }},
        { LenovoLegionDaemon::KEY_EN_PAGE_UP,           { "PUp"   , "PUp",              }},
        { LenovoLegionDaemon::KEY_EN_TAB,               { "Tab"   , "\xE2\x86\xb9"      }}, // ⭾ ↹ ⇄ ⇥
        { LenovoLegionDaemon::KEY_EN_DELETE,            { "Del"   , "Del",              }},
        { LenovoLegionDaemon::KEY_EN_END,               { "End"   , "End",              }},
        { LenovoLegionDaemon::KEY_EN_PAGE_DOWN,         { "PDn"   , "PDn",              }},
        { LenovoLegionDaemon::KEY_EN_CAPS_LOCK,         { "Cap"   , "Cap",              }},
        { LenovoLegionDaemon::KEY_EN_ANSI_ENTER,        { "Ent"   , "\xE2\x86\xB5"      }}, // ↵ ⏎
        { LenovoLegionDaemon::KEY_EN_ISO_ENTER,         { "Ent"   , "\xE2\x86\xB5"      }}, // ↵ ⏎
        { LenovoLegionDaemon::KEY_EN_LEFT_SHIFT,        { "Sft"   , "\xE2\x87\xA7"      }}, // ⇧
        { LenovoLegionDaemon::KEY_EN_RIGHT_SHIFT,       { "Sft"   , "\xE2\x87\xA7"      }}, // ⇧
        { LenovoLegionDaemon::KEY_EN_UP_ARROW,          { "Up"    , "\xF0\x9F\xA1\xB9"  }}, // ↑ 🡹
        { LenovoLegionDaemon::KEY_EN_LEFT_CONTROL,      { "Ctl"   , "Ctl",              }},
        { LenovoLegionDaemon::KEY_EN_LEFT_WINDOWS,      { "Win"   , "\xe2\x9d\x96"      }}, // ❖
        { LenovoLegionDaemon::KEY_EN_LEFT_FUNCTION,     { "Fn"    , "Fn",               }},
        { LenovoLegionDaemon::KEY_EN_LEFT_ALT,          { "Alt"   , "Alt",              }},
        { LenovoLegionDaemon::KEY_EN_SPACE,             { "Spc"   , "Spc",              }},
        { LenovoLegionDaemon::KEY_EN_RIGHT_ALT,         { "Alt"   , "Alt",              }},
        { LenovoLegionDaemon::KEY_EN_RIGHT_FUNCTION,    { "Fn"    , "Fn",               }},
        { LenovoLegionDaemon::KEY_EN_RIGHT_WINDOWS,     { "Win"   , "\xe2\x9d\x96"      }}, // ❖
        { LenovoLegionDaemon::KEY_EN_MENU,              { "Mnu"   , "\xE2\x98\xB0"      }}, // ▤ ☰ 𝌆 🗏
        { LenovoLegionDaemon::KEY_EN_RIGHT_CONTROL,     { "Ctl"   , "Ctl",              }},
        { LenovoLegionDaemon::KEY_EN_LEFT_ARROW,        { "Lft"   , "\xF0\x9F\xA1\xB8"  }}, // ↑ 🡹
        { LenovoLegionDaemon::KEY_EN_DOWN_ARROW,        { "Dn"    , "\xF0\x9F\xA1\xBB"  }}, // ↑ 🡹
        { LenovoLegionDaemon::KEY_EN_RIGHT_ARROW,       { "Rgt"   , "\xF0\x9F\xA1\xBA"  }}, // ↑ 🡹
        { LenovoLegionDaemon::KEY_EN_NUMPAD_LOCK,       { "Num"   , "Num",              }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_DIVIDE,     { "/"     , "/",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_TIMES,      { "*"     , "*",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_MINUS,      { "-"     , "-",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_PLUS,       { "+"     , "+",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_PERIOD,     { "."     , ".",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_ENTER,      { "Ent"   , "\xE2\x86\xB5"      }}, // ↵ ⏎
        { LenovoLegionDaemon::KEY_EN_NUMPAD_0,          { "0"     , "0",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_1,          { "1"     , "1",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_2,          { "2"     , "2",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_3,          { "3"     , "3",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_4,          { "4"     , "4",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_5,          { "5"     , "5",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_6,          { "6"     , "6",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_7,          { "7"     , "7",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_8,          { "8"     , "8",                }},
        { LenovoLegionDaemon::KEY_EN_NUMPAD_9,          { "9"     , "9",                }},
        { LenovoLegionDaemon::KEY_EN_MEDIA_PLAY_PAUSE,  { "Ply"   , "\xE2\x8F\xAF",     }}, // ⏯
        { LenovoLegionDaemon::KEY_EN_MEDIA_PREVIOUS,    { "Prv"   , "\xE2\x8F\xAE",     }}, // ⏮
        { LenovoLegionDaemon::KEY_EN_MEDIA_NEXT,        { "Nxt"   , "\xE2\x8F\xAD",     }}, // ⏭
        { LenovoLegionDaemon::KEY_EN_MEDIA_STOP,        { "Stp"   , "\xE2\x8F\xB9",     }}, // ⏹
        { LenovoLegionDaemon::KEY_EN_MEDIA_MUTE,        { "Mte"   , "\xF0\x9F\x94\x87", }}, // 🔇
        { LenovoLegionDaemon::KEY_EN_MEDIA_VOLUME_DOWN, { "Vl-"   , "\xF0\x9F\x94\x88", }}, // 🔉
        { LenovoLegionDaemon::KEY_EN_MEDIA_VOLUME_UP,   { "Vl+"   , "\xF0\x9F\x94\x89", }}, // 🔊
        { LenovoLegionDaemon::KEY_EN_POWER,             { "Pwr"   , "\xE2\x8F\xBB",     }}, // ⏻
        { LenovoLegionDaemon::KEY_JP_RO,                { "_"     , "_",                }},
        { LenovoLegionDaemon::KEY_JP_EJ,                { "E/J"   , "E/J",              }},
        { LenovoLegionDaemon::KEY_JP_ZENKAKU,           { "Zen"   , "\xE5\x8D\x8A\xE8"
                             "\xA7\x92\x2F\xE5"
                             "\x85\xA8\xE8\xA7"
                             "\x92",             }}, // 半角/全角
        { LenovoLegionDaemon::KEY_JP_KANA,              { "Kana"  , "\xE3\x81\x8B\xE3"
                          "\x81\xAA",         }}, // かな
        { LenovoLegionDaemon::KEY_JP_HENKAN,            { "Hnkn"  , "\xE5\xA4\x89\xE6"
                            "\x8F\x9B",         }}, // 変換
        { LenovoLegionDaemon::KEY_JP_MUHENKAN,          { "MuKn"  , "\xE7\x84\xA1\xE5"
                              "\xA4\x89\xE6\x8F"
                              "\x9B",             }}, // 無変換
        { LenovoLegionDaemon::KEY_JP_YEN,               { "Yen"   , "\xC2\xA5",         }}, // ¥
        { LenovoLegionDaemon::KEY_JP_AT,                { "@"     , "@",                }},
        { LenovoLegionDaemon::KEY_JP_CHEVRON,           { "^"     , "^",                }},
        { LenovoLegionDaemon::KEY_JP_COLON,             { ":"     , ":",                }},
        { LenovoLegionDaemon::KEY_KR_HAN,               { "Hayg"  , "\xED\x95\x9C\x2F"
                         "\xEC\x98\x81",     }}, // 한/영
        { LenovoLegionDaemon::KEY_KR_HANJA,             { "Haja"  , "\xED\x95\x9C\xEC"
                           "\x9E\x90",         }}, // 한자
        { LenovoLegionDaemon::KEY_NORD_AAL,             { "Å"     , "\xC3\x85",         }}, // Å
        { LenovoLegionDaemon::KEY_NORD_A_OE,            { "Ä Ø"   , "\xC3\x84\x20\xC3"
                                "\x98"              }}, // Ä Ø
        { LenovoLegionDaemon::KEY_NORD_O_AE,            { "Ö Æ"   , "\xC3\x96\x20\xC3"
                                "\x86"              }}, // Ö Æ
        { LenovoLegionDaemon::KEY_NORD_HALF,            { "§ ½"   , "\xC2\xA7\x20\xC2"
                                "\xBD"              }}, // § ½
        { LenovoLegionDaemon::KEY_NORD_HYPHEN,          { "- _"   , "- _"               }},
        { LenovoLegionDaemon::KEY_NORD_PLUS_QUESTION,   { "+ ?"   , "+ ?",              }},
        { LenovoLegionDaemon::KEY_NORD_ACUTE_GRAVE,     { "´ `"   , "\xC2\xB4\x20\x60", }}, // ´ `
        { LenovoLegionDaemon::KEY_NORD_DOTS_CARET,      { "¨ ^"   , "\xC2\xA8\x20\x5E", }}, // ¨ ^
        { LenovoLegionDaemon::KEY_NORD_QUOTE,           { "' *"   , "' *",              }},
        { LenovoLegionDaemon::KEY_NORD_ANGLE_BRACKET,   { "< >"   , "< >"               }},
        { LenovoLegionDaemon::KEY_DE_ESZETT,            { "ß"     , "\xc3\x9F",         }},
        { LenovoLegionDaemon::KEY_DE_DIAERESIS_A,       { "Ä"     , "\xC3\x84",         }},
        { LenovoLegionDaemon::KEY_DE_DIAERESIS_O,       { "Ö"     , "\xC3\x96",         }},
        { LenovoLegionDaemon::KEY_DE_DIAERESIS_U,       { "Ü"     , "\xC3\x9C"          }},
        { LenovoLegionDaemon::KEY_FR_SUPER_2,           { "²"     , "\xc2\xb2"          }},
        { LenovoLegionDaemon::KEY_FR_AMPERSAND,         { "&"     , "&"                 }},
        { LenovoLegionDaemon::KEY_FR_ACUTE_E,           { "é"     , "\xc3\xa9"          }},
        { LenovoLegionDaemon::KEY_FR_DOUBLEQUOTE,       { "\""    , "\""                }},
        { LenovoLegionDaemon::KEY_FR_LEFT_PARENTHESIS,  { "("     , "("                 }},
        { LenovoLegionDaemon::KEY_FR_GRAVE_E,           { "è"     , "\xc3\xa8"          }},
        { LenovoLegionDaemon::KEY_FR_UNDERSCORE,        { "_"     , "_"                 }},
        { LenovoLegionDaemon::KEY_FR_CEDILLA_C,         { "ç"     , "\xc3\xa7"          }},
        { LenovoLegionDaemon::KEY_FR_GRAVE_A,           { "à"     , "\xc3\xa0"          }},
        { LenovoLegionDaemon::KEY_FR_RIGHT_PARENTHESIS, { ")"     , ")"                 }},
        { LenovoLegionDaemon::KEY_FR_DOLLAR,            { "$"     , "$"                 }},
        { LenovoLegionDaemon::KEY_FR_GRAVE_U,           { "ù"     , "\xc3\xb9"          }},
        { LenovoLegionDaemon::KEY_FR_ASTERIX,           { "*"     , "*"                 }},
        { LenovoLegionDaemon::KEY_FR_EXCLAIMATION,      { "!"     , "!"                 }},
        { LenovoLegionDaemon::KEY_ES_OPEN_QUESTION_MARK,{ "¿"     , "¡"                 }},
        { LenovoLegionDaemon::KEY_ES_TILDE,             { "´"     , "¨"                 }},
        { LenovoLegionDaemon::KEY_ES_ENIE,              { "ñ"     , "Ñ"                 }},
        };

void DeviceView::setController(LenovoLegionDaemon::RGBControllerInterface *controller_ptr,const uint32_t vendorId,const uint32_t productId)
{
    /*-----------------------------------------------------*\
    | Store the controller pointer                          |
    \*-----------------------------------------------------*/
    controller = controller_ptr;


    if(vendorId == 0x48d && productId == 0xc197)
    {
        m_keyboardBackgroundImage = "keyboard-background-048d-c197.png";
    }

    if(vendorId == 0x48d && ((productId >> 8) & 0xFF) == 0xc9)
    {
        m_keyboardBackgroundImage = "keyboard-background-048d-c9xx.png";
    }
}

void DeviceView::InitDeviceView()
{
    /*-----------------------------------------------------*\
    | Set the size of the selection flags vector            |
    \*-----------------------------------------------------*/
    selectionFlags.resize((int)controller->GetLEDs().size());

    /*-----------------------------------------------------*\
    | Set the size of the zone and LED position vectors     |
    \*-----------------------------------------------------*/
    zone_pos.resize(controller->GetZones().size());
    led_pos.resize(controller->GetLEDs().size());
    led_labels.resize(controller->GetLEDs().size());
    led_colors.resize(controller->GetLEDs().size());
    led_to_color_map.clear();

    /*-----------------------------------------------------*\
    | Process position and size for zones                   |
    \*-----------------------------------------------------*/
    unsigned int maxWidth       = 0;
    unsigned int segment_count  = 0;
    float        totalHeight    = 0.0f;

    /*-----------------------------------------------------*\
    | Get device view settings                              |
    \*-----------------------------------------------------*/
    std::string         ui_string           = "UserInterface";

    bool                disable_expansion   = false;


    /*-----------------------------------------------------*\
    | Determine the total height (in LEDs) of all zones     |
    \*-----------------------------------------------------*/
    for(std::size_t zone_idx = 0; zone_idx < controller->GetZones().size(); zone_idx++)
    {
        /*-----------------------------------------------------*\
        | For matrix zones, use matrix height from the map      |
        \*-----------------------------------------------------*/
        if((controller->GetZones()[zone_idx].type == LenovoLegionDaemon::ZONE_TYPE_MATRIX) && (controller->GetZones()[zone_idx].matrix_map.map.size() > 0))
        {
            totalHeight                += controller->GetZones()[zone_idx].matrix_map.height;
            zone_pos[zone_idx].matrix_w = controller->GetZones()[zone_idx].matrix_map.width;
        }
        else
        {
            unsigned int count          = controller->GetLEDsInZone(zone_idx);
            zone_pos[zone_idx].matrix_w = std::min(count, (unsigned int)MAX_COLS);
            totalHeight                += (count / MAX_COLS) + ((count % MAX_COLS) > 0);
        }

        /*-----------------------------------------------------*\
        | Determine the maximum width (in LEDs) in the view     |
        \*-----------------------------------------------------*/
        if(zone_pos[zone_idx].matrix_w > maxWidth)
        {
            maxWidth = zone_pos[zone_idx].matrix_w;
        }
    }

    segment_pos.resize(segment_count);

    /*-----------------------------------------------------*\
    | Add some space for zone names and padding             |
    \*-----------------------------------------------------*/
    totalHeight    += controller->GetZones().size() * PAD_ZONE;
    totalHeight    += segment_count * PAD_SEGMENT;

    float current_y = 0;                    // We will be descending, placing each zone one unit below the previous one
    matrix_h        = totalHeight;

    for(std::size_t zone_idx = 0; zone_idx < controller->GetZones().size(); zone_idx++)
    {
        /*-----------------------------------------------------*\
        | Calculate zone label position and size                |
        \*-----------------------------------------------------*/
        zone_pos[zone_idx].matrix_x = (maxWidth - zone_pos[zone_idx].matrix_w) / 2.0f;
        zone_pos[zone_idx].matrix_y = current_y + SIZE_TEXT;
        zone_pos[zone_idx].matrix_h = SIZE_TEXT - PAD_TEXT;
        current_y                  += PAD_ZONE;

        /*-----------------------------------------------------*\
        | Calculate LEDs position and size for zone             |
        \*-----------------------------------------------------*/
        if((controller->GetZones()[zone_idx].type == LenovoLegionDaemon::ZONE_TYPE_MATRIX) && (controller->GetZones()[zone_idx].matrix_map.map.size()))
        {
            LenovoLegionDaemon::matrix_map_type  localMap = controller->GetZones()[zone_idx].matrix_map;
            LenovoLegionDaemon::matrix_map_type* map      = &localMap;


            for(unsigned int led_x = 0; led_x < map->width; led_x++)
            {
                for(unsigned int led_y = 0; led_y < map->height; led_y++)
                {
                    unsigned int map_idx    = led_y * map->width + led_x;

                    if((map_idx + 1) < map->map.size()  && controller->GetLEDName(map->map[map_idx + 1]) == LenovoLegionDaemon::KEY_EN_RIGHT_ARROW && map->map[map_idx] == 0xFFFFFFFF)
                    {
                        std::swap(map->map[map_idx], map->map[map_idx + 1]);
                    }

                    if((map_idx + 2) < map->map.size() && controller->GetLEDName(map->map[map_idx])  == LenovoLegionDaemon::KEY_EN_TAB &&  map->map[map_idx + 2] == 0xFFFFFFFF && (map_idx + 1))
                    {
                        std::swap(map->map[map_idx + 1], map->map[map_idx + 2]);
                    }

                    unsigned int color_idx  = map->map[map_idx] + controller->GetZones()[zone_idx].start_idx;

                    if(map->map[map_idx] != 0xFFFFFFFF && color_idx < led_pos.size())
                    {
                        led_pos[color_idx].matrix_x = (zone_pos[zone_idx].matrix_x + led_x + PAD_LED);
                        led_pos[color_idx].matrix_y = current_y + (led_y + PAD_LED);

                        /*-----------------------------------------------------*\
                        | LED is a 1x1 square, minus padding on all sides       |
                        \*-----------------------------------------------------*/
                        led_pos[color_idx].matrix_w = (1.0f - (2.0f * PAD_LED));
                        led_pos[color_idx].matrix_h = (1.0f - (2.0f * PAD_LED));

                        if(!disable_expansion)
                        {


                            /*-----------------------------------------------------*\
                            | Expand large keys to fill empty spaces in matrix, if  |
                            | possible.  Large keys can fill left, down, up, or wide|
                            | Fill Left:                                            |
                            |    Tab                                                |
                            |    Caps Lock                                          |
                            |    Left Shift                                         |
                            |    Right Shift                                        |
                            |    Backspace                                          |
                            |    Number Pad 0                                       |
                            |    ANSI Enter                                         |
                            | Fill Up or Down:                                      |
                            |    Number Pad Enter                                   |
                            |    Number Pad                                         |
                            |    ISO Enter                                          |
                            | Fill Wide:                                            |
                            |    Space                                              |
                            \*-----------------------------------------------------*/
                            if( ( controller->GetLEDName(color_idx)    == LenovoLegionDaemon::KEY_EN_TAB        )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_CAPS_LOCK  )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_LEFT_SHIFT )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_RIGHT_SHIFT)
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_BACKSPACE  )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_NUMPAD_0   )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_ANSI_ENTER))
                            {

                                if(led_x > 0 && controller->GetLEDName(map->map[map_idx - 1] + controller->GetZones()[zone_idx].start_idx)  == controller->GetLEDName(color_idx))
                                {
                                    led_pos[color_idx].matrix_x -= 1.0f;
                                    led_pos[color_idx].matrix_w += 1.0f;
                                } else if(led_x < map->width - 1 && (controller->GetLEDName(map->map[map_idx + 1] + controller->GetZones()[zone_idx].start_idx)  == controller->GetLEDName(color_idx) || map->map[map_idx + 1] == 0xFFFFFFFF))
                                {
                                    led_pos[color_idx].matrix_w += 1.0f;
                                }
                            }


                            if( ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_NUMPAD_ENTER   )
                                || ( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_NUMPAD_PLUS    ) )
                            {
                                if(led_y > 0 && controller->GetLEDName(map->map[map_idx - map->width ] + controller->GetZones()[zone_idx].start_idx)  == controller->GetLEDName(color_idx))
                                {
                                    led_pos[color_idx].matrix_y -= 1.0f;
                                    led_pos[color_idx].matrix_h += 1.0f;
                                }
                                else if(led_y < map->height - 1 && controller->GetLEDName(map->map[map_idx + map->width ] + controller->GetZones()[zone_idx].start_idx)  == controller->GetLEDName(color_idx))
                                {
                                    led_pos[color_idx].matrix_h += 1.0f;
                                }
                            }


                            if( controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_ISO_ENTER)
                            {
                                if(led_y > 0 && controller->GetLEDName(map->map[map_idx - map->width ] + controller->GetZones()[zone_idx].start_idx) == controller->GetLEDName(color_idx))
                                {
                                    led_pos[color_idx].matrix_y -= 1.0f;
                                    led_pos[color_idx].matrix_h += 1.0f;
                                }
                            }



                            if(controller->GetLEDName(color_idx) == LenovoLegionDaemon::KEY_EN_SPACE)
                            {
                                for(unsigned int map_idx2 = map_idx - 1; map_idx2 > led_y * map->width && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2--)
                                {
                                    led_pos[color_idx].matrix_x -= 1.0f;
                                    led_pos[color_idx].matrix_w += 1.0f;
                                }
                                for(unsigned int map_idx2 = map_idx + 1; map_idx2 < (led_y + 1) * map->width && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2++)
                                {
                                    led_pos[color_idx].matrix_w += 1.0f;
                                }
                            }

                            if(!LenovoLegionDaemon::KeyCodesToName.contains(controller->GetLEDs().at(color_idx).value))
                            {
                                /*
                                 * Horizontal
                                 */
                                for(unsigned int map_idx2 = map_idx - 1; map_idx2 >= led_y * map->width && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2--)
                                {
                                    led_pos[color_idx].matrix_x -= 1.0f;
                                    led_pos[color_idx].matrix_w += 1.0f;
                                }
                                for(unsigned int map_idx2 = map_idx + 1; map_idx2 < (led_y + 1) * map->width && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2++)
                                {
                                    led_pos[color_idx].matrix_w += 1.0f;
                                }

                                /*
                                 * Vertical
                                 */
                                for(unsigned int map_idx2 = map_idx - map->width; map_idx2 < map->map.size() && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2 -= map->width)
                                {
                                    led_pos[color_idx].matrix_y -= 1.0f;
                                    led_pos[color_idx].matrix_h += 1.0f;
                                }

                                for(unsigned int map_idx2 = map_idx + map->width; map_idx2 < map->map.size() && controller->GetLEDName(color_idx) == controller->GetLEDName(map->map[map_idx2] + controller->GetZones()[zone_idx].start_idx); map_idx2 += map->width)
                                {
                                    led_pos[color_idx].matrix_h += 1.0f;
                                }

                            }
                        }

                    }
                }
            }

            current_y += map->height;
        }
        else
        {
            /*-----------------------------------------------------*\
            | Calculate LED box positions for single/linear zones   |
            \*-----------------------------------------------------*/
            unsigned int leds_count = controller->GetLEDsInZone(zone_idx);

            for(unsigned int led_idx = 0; led_idx < leds_count; led_idx++)
            {
                unsigned int led_pos_idx = controller->GetZones()[zone_idx].start_idx + led_idx;

                led_pos[led_pos_idx].matrix_x = zone_pos[zone_idx].matrix_x + ((led_idx % MAX_COLS) + PAD_LED);
                led_pos[led_pos_idx].matrix_y = current_y + ((led_idx / MAX_COLS) + PAD_LED);

                /*-----------------------------------------------------*\
                | LED is a 1x1 square, minus padding on all sides       |
                \*-----------------------------------------------------*/
                led_pos[led_pos_idx].matrix_w = (1.0f - (2.0f * PAD_LED));
                led_pos[led_pos_idx].matrix_h = (1.0f - (2.0f * PAD_LED));

                if(!disable_expansion)
                {
                    /*-----------------------------------------------------*\
                     | Merge same named LEDs into larger rectangles         |
                     *-----------------------------------------------------*/
                    for (unsigned int led_i = 1; (led_idx + led_i) < leds_count ; ++led_i)
                    {
                        if(controller->GetLEDName(led_pos_idx) == controller->GetLEDName(led_pos_idx + led_i))
                        {
                            led_pos[led_pos_idx].matrix_w += 1.0f;
                        }
                        else
                        {
                            break;
                        }
                    }

                    for (int led_i = 1; (static_cast<int>(led_idx) - led_i) >= 0 ; ++led_i)
                    {
                        if(controller->GetLEDName(led_pos_idx) == controller->GetLEDName(led_pos_idx - led_i))
                        {
                            led_pos[led_pos_idx].matrix_x -= 1.0f;
                            led_pos[led_pos_idx].matrix_w += 1.0f;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
            }

            current_y += (leds_count / MAX_COLS) + ((leds_count % MAX_COLS) > 0);
        }
    }

    /*-----------------------------------------------------*\
    | Update LED labels                                     |
    \*-----------------------------------------------------*/
    for(std::size_t led_idx = 0; led_idx < controller->GetLEDs().size(); led_idx++)
    {
        std::map<std::string, led_label>::const_iterator it = led_label_lookup.find(controller->GetLEDName(led_idx));

        if(it != led_label_lookup.end())
        {
            led_labels[led_idx] = it->second.label_utf8;
        }
        else if(numerical_labels)
        {
            led_labels[led_idx] = QString::number(led_idx);
        }
    }

    /*-----------------------------------------------------*\
    | Scale the zones, segments, and LEDs                   |
    |                                                       |
    | Atom is the width of a single square; if the whole    |
    | thing becomes too tall, we ignore it and let the view |
    | widget take care of it                                |
    \*-----------------------------------------------------*/
    float atom = 1.0f / maxWidth;

    for(std::size_t zone_idx = 0; zone_idx < zone_pos.size(); zone_idx++)
    {
        zone_pos[zone_idx].matrix_x *= atom;
        zone_pos[zone_idx].matrix_y *= atom;
        zone_pos[zone_idx].matrix_w *= atom;
        zone_pos[zone_idx].matrix_h *= atom;
    }

    for(std::size_t segment_idx = 0; segment_idx < segment_pos.size(); segment_idx++)
    {
        segment_pos[segment_idx].matrix_x *= atom;
        segment_pos[segment_idx].matrix_y *= atom;
        segment_pos[segment_idx].matrix_w *= atom;
        segment_pos[segment_idx].matrix_h *= atom;
    }

    for(std::size_t led_idx = 0; led_idx < led_pos.size(); led_idx++)
    {
        led_pos[led_idx].matrix_x *= atom;
        led_pos[led_idx].matrix_y *= atom;
        led_pos[led_idx].matrix_w *= atom;
        led_pos[led_idx].matrix_h *= atom;
    }

    matrix_h *= atom;

    /*-----------------------------------------------------*\
    | Update cached size and offset                         |
    \*-----------------------------------------------------*/
    size     = width();
    offset_x = 0;

    if(height() < size * matrix_h)
    {
        size     = height() / matrix_h;
        offset_x = (width() - size) / 2;
    }
}

void DeviceView::setNumericalLabels(bool enable)
{
    numerical_labels = enable;
}

void DeviceView::setPerLED(bool per_led_mode)
{
    per_led = per_led_mode;
    update();
}

void DeviceView::markLeds(const  QMap<int,QColor> &leds)
{
    for(auto item = leds.begin();item != leds.end(); ++item)
    {
        if(item.key() < 0 || size_t(item.key()) >= controller->GetLEDs().size())
        {
            return;
        }
    }

    marketLeds = leds;

    update();
}

void DeviceView::cleanup()
{
    if(m_timerId != -1)
    {
        killTimer(m_timerId);
        m_timerId = -1;
    }
}

QSize DeviceView::sizeHint () const
{
    return QSize(height() - 1, height() - 1);
}

QSize DeviceView::minimumSizeHint () const
{
    return initSize;
}

void DeviceView::mousePressEvent(QMouseEvent *event)
{
    if(per_led)
    {
        ctrlDown    = event->modifiers().testFlag(Qt::ControlModifier);
        mouseDown   = true;
        mouseMoved  = false;

        if(ctrlDown)
        {
            previousFlags = selectionFlags;
            previousSelection = selectedLeds;
        }

        /*-----------------------------------------------------*\
        | It's okay if the size becomes negative                |
        \*-----------------------------------------------------*/
        selectionRect.setLeft(event->position().x());
        selectionRect.setTop(event->position().y());
        selectionRect.setRight(event->position().x());
        selectionRect.setBottom(event->position().y());

        updateSelection();
        update();
    }
}

void DeviceView::mouseMoveEvent(QMouseEvent *event)
{
    if(per_led)
    {
        lastMousePos = event->pos();
        selectionRect.setRight(event->position().x());
        selectionRect.setBottom(event->position().y());

        if(mouseDown)
        {
            mouseMoved  = true;
            ctrlDown    = event->modifiers().testFlag(Qt::ControlModifier);

            /*-----------------------------------------------------*\
            | Clear the previous selection in case ctrl is released |
            \*-----------------------------------------------------*/
            if(!ctrlDown)
            {
                previousSelection.clear();
                previousFlags.clear();
                previousFlags.resize((int)controller->GetLEDs().size());
            }
            updateSelection();
        }
        update();
    }
}

void DeviceView::mouseReleaseEvent(QMouseEvent* event)
{
    if(per_led)
    {
        selectionRect = selectionRect.normalized();
        mouseDown = false;

        /*-----------------------------------------------------*\
        | Check if the user clicked a zone name & select it     |
        \*-----------------------------------------------------*/
        if(!mouseMoved)
        {
            int size     = width();
            int offset_x = 0;

            if(height() < size * matrix_h)
            {
                size     = height() / matrix_h;
                offset_x = (width() - size) / 2;
            }

            for(unsigned int zone_idx = 0; zone_idx < controller->GetZones().size(); zone_idx++)
            {
                int posx = zone_pos[zone_idx].matrix_x * size + offset_x + 12;
                int posy = zone_pos[zone_idx].matrix_y * size;
                int posw = zone_pos[zone_idx].matrix_w * size;
                int posh = zone_pos[zone_idx].matrix_h * size;

                QRect rect = {posx, posy, posw, posh};

                if(rect.contains(event->pos()))
                {
                    selectZone(zone_idx, ctrlDown);
                }
            }
        }
        update();
    }
}

void DeviceView::resizeEvent(QResizeEvent* /*event*/)
{
    size     = width();
    offset_x = 0;

    if(height() < size * matrix_h)
    {
        size     = height() / matrix_h;
        offset_x = (width() - size) / 2;
    }
    update();
}

void DeviceView::paintEvent(QPaintEvent* /* event */)
{
    QPainter painter(this);
    QFont font = painter.font();

    /*-----------------------------------------------------*\
    | If Device View is hidden, don't paint                 |
    \*-----------------------------------------------------*/
    if(isHidden())
    {
        return;
    }

    /*-----------------------------------------------------*\
    | If controller has resized, reinitialize local data    |
    \*-----------------------------------------------------*/
    if(controller->GetLEDs().size() != led_pos.size())
    {
        InitDeviceView();
    }


    /*-----------------------------------------------------*\
     * Draw background image                                *
     * -----------------------------------------------------*/
    QPixmap backgroundPixmap = QPixmap(QString(":/images/").append(m_keyboardBackgroundImage)).scaled(size + 50,height() - 10 , Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    painter.drawPixmap((width() - backgroundPixmap.width()) / 2, (height() - backgroundPixmap.height()) / 2, backgroundPixmap);

    /*-----------------------------------------------------*\
    | LED rectangles                                        |
    \*-----------------------------------------------------*/
    for(unsigned int led_idx = 0; led_idx < controller->GetLEDs().size(); led_idx++)
    {
        int posx = led_pos[led_idx].matrix_x * size + offset_x;
        int posy = led_pos[led_idx].matrix_y * size;
        int posw = led_pos[led_idx].matrix_w * size;
        int posh = led_pos[led_idx].matrix_h * size;

        QRect rect = {posx, posy, posw, posh};

        /*-----------------------------------------------------*\
        | Fill color                                            |
        \*-----------------------------------------------------*/
        QColor currentColor = QColor();

        if(led_to_color_map.contains(led_idx))
        {
            currentColor = led_to_color_map[led_idx];
        }
        else if(led_colors.size() > led_idx)
        {
            currentColor = QColor::fromRgb(RGBGetRValue(led_colors[led_idx]),RGBGetGValue(led_colors[led_idx]),RGBGetBValue(led_colors[led_idx]));
        }
        else
        {
            currentColor = QColor::fromRgb(0,0,0);
        }

        painter.setBrush(currentColor);

        /*
         * Market LEDs (when in per-LED mode) with a distinct border
         */
        if(marketLeds.contains(static_cast<int>(led_idx)))
        {
            QRect rect = { posx , posy, posw , posh };

            QPen pen1(QColor(marketLeds[led_idx]), 15);

            QPen currentPen = painter.pen();

            painter.setPen(pen1);

            painter.drawRect(rect);

            painter.setPen(currentPen);
        }

        /*-----------------------------------------------------*\
        | Border color                                          |
        \*-----------------------------------------------------*/
        if(selectionFlags[led_idx])
        {
            QPen innerPen(Qt::gray);
            innerPen.setWidth(4);
            painter.setPen(innerPen);
        }
        else
        {
            QPen defaultPen(QColor(49, 49 , 49, 160));
            defaultPen.setWidth(3);
            painter.setPen(defaultPen);
        }

        painter.drawRect(rect);

        /*-----------------------------------------------------*\
        | Label                                                 |
        | Set the font color so that the text is visible        |
        \*-----------------------------------------------------*/
        font.setPixelSize(posh / 2);
        painter.setFont(font);

        unsigned int luma = (unsigned int)(0.2126f * currentColor.red() + 0.7152f * currentColor.green() + 0.0722f * currentColor.blue());

        if(luma > 127)
        {
            painter.setPen(Qt::black);
        }
        else
        {
            painter.setPen(Qt::white);
        }
        painter.drawText(rect, Qt::AlignVCenter | Qt::AlignHCenter, QString(led_labels[led_idx]));
    }

    font.setPixelSize(12);
    painter.setFont(font);

    /*-----------------------------------------------------*\
    | Zone and Segment names                                |
    \*-----------------------------------------------------*/
    for(std::size_t zone_idx = 0; zone_idx < controller->GetZones().size(); zone_idx++)
    {
        int posx = zone_pos[zone_idx].matrix_x * size + offset_x;
        int posy = zone_pos[zone_idx].matrix_y * size;
        int posw = zone_pos[zone_idx].matrix_w * size;
        int posh = zone_pos[zone_idx].matrix_h * size;

        QRect rect = {posx, posy, posw, posh};

        if(rect.contains(lastMousePos) && (!mouseDown || !mouseMoved))
        {
            painter.setPen(palette().highlight().color());
        }
        else
        {
            painter.setPen(palette().windowText().color());
        }
        painter.drawText(posx, posy + posh, QString(controller->GetZoneName(zone_idx).c_str()));
    }

    /*-----------------------------------------------------*\
    | Selection area                                        |
    \*-----------------------------------------------------*/
    if(mouseDown)
    {
        QRect rect = selectionRect.normalized();
        QColor color = palette().highlight().color();
        color.setAlpha(63);
        painter.fillRect(rect, color);
        color.setAlpha(175);
        painter.setBrush(color);
        painter.drawRect(rect);
    }
}

void DeviceView::timerEvent(QTimerEvent *)
{
    try {
        if(controller && isVisible())
        {
            led_colors = controller->GetStateForAllLeds();
            update();
        }
    } catch(const ProtocolProcessorBase::exception_T &ex)
    {
        LOG_W(QString::asprintf("DeviceView: Caught exception while updating LED colors: %s", ex.what()));
    }
}

void DeviceView::updateSelection()
{
    selectedLeds.clear();
    selectionFlags.clear();
    selectionFlags.resize((int)controller->GetLEDs().size());

    QRect sel              = selectionRect.normalized();
    std::vector<LenovoLegionDaemon::led> leds  = controller->GetLEDs();

    for(unsigned int led_idx = 0; led_idx < leds.size(); led_idx++)
    {
        /*-----------------------------------------------------*\
        | Check intersection                                    |
        \*-----------------------------------------------------*/
        int posx = led_pos[led_idx].matrix_x * size + offset_x;
        int posy = led_pos[led_idx].matrix_y * size;
        int posw = led_pos[led_idx].matrix_w * size;
        int posh = led_pos[led_idx].matrix_h * size;

        QRect rect = {posx, posy, posw, posh};

        selectionFlags[led_idx] = 0;

        if(sel.intersects(rect))
        {
            selectionFlags[led_idx] = 1;
        }
        if(ctrlDown)
        {
            selectionFlags[led_idx] ^= previousFlags[led_idx];
        }

        if(selectionFlags[led_idx])
        {
            selectedLeds.push_back(led_idx);
        }
    }

    update();

    /*-----------------------------------------------------*\
    | Send selection changed signal                         |
    \*-----------------------------------------------------*/
    emit selectionChanged(selectedLeds);
}

bool DeviceView::selectLed(int target)
{
    if(target < 0 || size_t(target) >= controller->GetLEDs().size())
    {
        return false;
    }

    selectedLeds.resize(1);
    selectedLeds[0] = target;
    selectionFlags.clear();
    selectionFlags.resize((int)controller->GetLEDs().size());
    selectionFlags[target] = 1;

    update();

    /*-----------------------------------------------------*\
    | Send selection changed signal                         |
    \*-----------------------------------------------------*/
    emit selectionChanged(selectedLeds);

    return true;
}

bool DeviceView::selectLeds(QVector<int> target)
{
    for(int item: target)
    {
        if(item < 0 || size_t(item) >= controller->GetLEDs().size())
        {
            return false;
        }
    }

    selectionFlags.clear();
    selectionFlags.resize((int)controller->GetLEDs().size());

    for(int item: target)
    {
        selectionFlags[item] = 1;
    }

    //selectedLeds = target;

    /*-----------------------------------------------------*\
    | Filter out duplicate items                            |
    \*-----------------------------------------------------*/
    selectedLeds.clear();

    for(int i = 0; i < selectionFlags.size(); ++i)
    {
        if(selectionFlags[i])
        {
            selectedLeds.push_back(i);
        }
    }

    update();

    /*-----------------------------------------------------*\
    | Send selection changed signal                         |
    \*-----------------------------------------------------*/
    emit selectionChanged(selectedLeds);

    return true;
}

bool DeviceView::selectZone(int zone, bool add)
{
    if(zone < 0 || size_t(zone) >= controller->GetZones().size())
    {
        return false;
    }

    if(!add)
    {
        selectedLeds.clear();
        selectionFlags.clear();
        selectionFlags.resize((int)controller->GetLEDs().size());
    }

    int zoneStart = controller->GetZones()[zone].start_idx;

    for(std::size_t led_idx = 0; led_idx < controller->GetLEDsInZone(zone); led_idx++)
    {
        if(!selectionFlags[zoneStart + led_idx])
        {
            selectedLeds.push_back(zoneStart + led_idx);
            selectionFlags[zoneStart + led_idx] = 1;
        }
    }

    update();

    /*-----------------------------------------------------*\
    | Send selection changed signal                         |
    \*-----------------------------------------------------*/
    emit selectionChanged(selectedLeds);

    return true;
}

void DeviceView::clearSelection()
{
    /*-----------------------------------------------------*\
    | Same as selecting the entire device                   |
    \*-----------------------------------------------------*/
    selectedLeds.clear();
    selectionFlags.clear();
    selectionFlags.resize((int)controller->GetLEDs().size());
}

void DeviceView::setSelectionColor(LenovoLegionDaemon::RGBColor color)
{
    led_to_color_map.clear();

    for(int led_idx : std::as_const(selectedLeds))
    {
        led_to_color_map[led_idx] = QColor::fromRgb(
            RGBGetRValue(color),
            RGBGetGValue(color),
            RGBGetBValue(color));
    }

    update();
}

}

