// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2026
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */
#include "KeyboardLayout.h"
#include "DataSource.h"

#include "../LenovoLegion-DaemonSDK/RGBControllerKeyNames.h"

#include "../LenovoLegion-PrepareBuild/RGBController.pb.h"

#include <QString>

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace LenovoLegionGui {

namespace {

constexpr unsigned int MAX_COLS    = 20;
constexpr float        PAD_LED     = 0.1f;
constexpr float        PAD_TEXT    = 0.1f;
constexpr float        PAD_ZONE    = 1.0f;
constexpr float        SIZE_TEXT   = 0.5f;
constexpr unsigned int NO_LED      = 0xFFFFFFFF;

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

struct Rect
{
    float x = 0, y = 0, w = 0, h = 0;
};

/*
 * Physical layout of the Lenovo Legion keyboards (Legion Pro 7i / 7i Gen 10, ITE 8258 048d:c197) by the key codes of the
 * LEDs (RGBControllerKeyNames.h KeyCodesToName), measured on the photo of the keyboard. The matrix map of the controller
 * only approximates the positions (holes simulate the shifted rows), the names are not unique (two "\ (ISO)").
 * Unit: pitch of the main block. The top row has 20 narrower keys over the whole width, the numpad is narrower.
 */
constexpr float KB_WIDTH  = 17.65f;     // keyboard (top row, main block + numpad)
constexpr float KB_HEIGHT = 6.6f;
constexpr float TOP_PITCH = KB_WIDTH / 20.0f;
constexpr float NUM_X     = 14.37f;     // start of the numpad
constexpr float NUM_PITCH = (KB_WIDTH - NUM_X) / 4.0f;
constexpr float ARROW_W   = 1.07f;

struct TemplateKey
{
    int     code;
    Rect    rect;
    QString label;      // empty: label of the name (led_label_lookup)
};

std::vector<TemplateKey> legionTemplate()
{
    std::vector<TemplateKey> keys;

    auto row = [&keys](std::initializer_list<int> codes,float x,float y,float pitch,float h) {
        for(int code : codes)
        {
            keys.push_back({ code, { x, y, pitch, h }, {} });
            x += pitch;
        }
    };

    auto key = [&keys](int code,float x,float y,float w,float h,const QString& label = {}) {
        keys.push_back({ code, { x, y, w, h }, label });
    };

    // Esc, F1 - F12, Insert, Print Screen, Delete, Home, End, Page Up, Page Down
    row({ 0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14 },0,0,TOP_PITCH,0.7f);

    // ` 1 - 0 - = Backspace | Num Lock / * -
    row({ 0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22 },0,0.7f,1,1);
    key(0x38,13,0.7f,NUM_X - 13,1);
    row({ 0x26,0x27,0x28,0x29 },NUM_X,0.7f,NUM_PITCH,1);

    // Tab Q - ] \ | 7 8 9 +   (\ of the ANSI keyboard is the LED of the ISO key left of Enter)
    key(0x40,0,1.7f,1.33f,1);
    row({ 0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D },1.33f,1.7f,1,1);
    key(0xA8,13.33f,1.7f,NUM_X - 13.33f,1,"\\");
    row({ 0x4F,0x50,0x51 },NUM_X,1.7f,NUM_PITCH,1);
    key(0x68,NUM_X + 3 * NUM_PITCH,1.7f,NUM_PITCH,2);

    // Caps Lock A - ' Enter | 4 5 6
    key(0x55,0,2.7f,1.63f,1);
    row({ 0x6D,0x6E,0x58,0x59,0x5A,0x71,0x72,0x5B,0x5C,0x5D,0x5F },1.63f,2.7f,1,1);
    key(0x77,12.63f,2.7f,NUM_X - 12.63f,1);
    row({ 0x79,0x7B,0x7C },NUM_X,2.7f,NUM_PITCH,1);

    // Left Shift (also the LED of the ISO key next to it) Z - / Right Shift | 1 2 3 Num Enter
    key(0x6A,0,3.7f,2.13f,1);
    key(0x4E,0,3.7f,2.13f,1);
    row({ 0x82,0x83,0x6F,0x70,0x87,0x88,0x73,0x74,0x75,0x76 },2.13f,3.7f,1,1);
    key(0x8D,12.13f,3.7f,NUM_X - 12.13f,1);
    row({ 0x8E,0x90,0x92 },NUM_X,3.7f,NUM_PITCH,1);
    key(0xA7,NUM_X + 3 * NUM_PITCH,3.7f,NUM_PITCH,2);

    // Ctrl Fn Win Alt Space Alt Copilot  Up | 0 .
    key(0x7F,0,4.7f,1.13f,1);
    row({ 0x80,0x96,0x97 },1.13f,4.7f,1,1);
    key(0x98,4.13f,4.7f,4.98f,1);
    key(0x9A,9.11f,4.7f,1,1);
    key(0x9B,10.11f,4.7f,1,1,"Copilot");
    key(0x9D,NUM_X - 2 * ARROW_W,4.7f,ARROW_W,1);
    key(0xA3,NUM_X,4.7f,2 * NUM_PITCH,1);
    key(0xA5,NUM_X + 2 * NUM_PITCH,4.7f,NUM_PITCH,1);

    // Left Down Right under the Up arrow
    row({ 0x9C,0x9F,0xA1 },NUM_X - 3 * ARROW_W,5.7f,ARROW_W,0.9f);

    return keys;
}

// Light bars of the chassis (LED codes without a key): rear around the vents (top row of the matrix), front along the
// bottom edge (around the matrix), logo on the lid
constexpr int  LOGO_CODE           = 1501;
constexpr int  MIN_TEMPLATE_KEYS   = 60;    // fewer keys found: not this keyboard, generic layout by the matrix
constexpr float LIGHT_THICKNESS    = 0.26f;

}

KeyboardLayout::KeyboardLayout(QObject *parent) : QObject(parent)
{}

DataSource *KeyboardLayout::source() const
{
    return m_source;
}

void KeyboardLayout::setSource(DataSource *source)
{
    if(source == m_source)
    {
        return;
    }

    if(m_source != nullptr)
    {
        disconnect(m_source,nullptr,this,nullptr);
    }

    m_source = source;

    if(m_source != nullptr)
    {
        connect(m_source,&DataSource::messageChanged,this,&KeyboardLayout::update);
    }

    emit sourceChanged();
    update();
}

QVariantList KeyboardLayout::keys() const
{
    return m_keys;
}

QVariantList KeyboardLayout::zones() const
{
    return m_zones;
}

qreal KeyboardLayout::heightRatio() const
{
    return m_heightRatio;
}

QVariantList KeyboardLayout::views() const
{
    return m_views;
}

QVariantList KeyboardLayout::keyLeds() const
{
    return m_keyLeds;
}

void KeyboardLayout::update()
{
    legion::messages::RGBControllerResponse data;

    if(m_source != nullptr && m_source->message() != nullptr && m_source->message()->GetDescriptor() == data.GetDescriptor())
    {
        data.CopyFrom(*m_source->message());
    }

    /*
     * Only LEDs and zones define the layout (colors, brightness, ... change often)
     */
    legion::messages::RGBControllerResponse layoutPart;
    *layoutPart.mutable_leds()  = data.leds();
    *layoutPart.mutable_zones() = data.zones();

    std::string serialized;
    if(!layoutPart.SerializeToString(&serialized))
    {
        return;
    }

    const QByteArray layoutData(serialized.data(),static_cast<qsizetype>(serialized.size()));

    if(layoutData == m_layoutData && !m_keys.isEmpty())
    {
        return;
    }

    m_layoutData = layoutData;

    if(updateByTemplate(data))
    {
        emit layoutChanged();
        return;
    }

    const auto& leds  = data.leds();
    const auto& zones = data.zones();

    auto name = [&leds](unsigned int led) -> std::string {
        return led < static_cast<unsigned int>(leds.size()) ? std::string(leds.at(static_cast<int>(led)).name()) : std::string();
    };

    std::vector<Rect> zonePos(static_cast<size_t>(zones.size()));
    std::vector<Rect> ledPos(static_cast<size_t>(leds.size()));
    std::vector<unsigned int> zoneMatrixW(static_cast<size_t>(zones.size()),0);

    /*
     * Maximum width (in LEDs) of the zones
     */
    unsigned int maxWidth    = 0;

    for(int zoneIdx = 0; zoneIdx < zones.size(); ++zoneIdx)
    {
        const auto& zone = zones.at(zoneIdx);

        if(zone.type() == legion::messages::RGBController::ZONE_TYPE_MATRIX && zone.matrix_map().map_size() > 0)
        {
            zoneMatrixW[zoneIdx]  = zone.matrix_map().width();
        }
        else
        {
            const unsigned int count = zone.leds_count();
            zoneMatrixW[zoneIdx]     = std::min(count,MAX_COLS);
        }

        maxWidth = std::max(maxWidth,zoneMatrixW[zoneIdx]);
    }

    float currentY = 0;

    for(int zoneIdx = 0; zoneIdx < zones.size(); ++zoneIdx)
    {
        const auto& zone = zones.at(zoneIdx);

        zonePos[zoneIdx] = { (maxWidth - zoneMatrixW[zoneIdx]) / 2.0f, currentY + SIZE_TEXT, static_cast<float>(zoneMatrixW[zoneIdx]), SIZE_TEXT - PAD_TEXT };
        currentY        += PAD_ZONE;

        if(zone.type() == legion::messages::RGBController::ZONE_TYPE_MATRIX && zone.matrix_map().map_size() > 0)
        {
            const unsigned int width  = zone.matrix_map().width();
            const unsigned int height = zone.matrix_map().height();
            std::vector<unsigned int> map(zone.matrix_map().map().begin(),zone.matrix_map().map().end());

            auto mapName = [&](unsigned int idx) { return idx < map.size() && map[idx] != NO_LED ? name(map[idx] + zone.start_idx()) : std::string(); };

            for(unsigned int ledX = 0; ledX < width; ++ledX)
            {
                for(unsigned int ledY = 0; ledY < height; ++ledY)
                {
                    const unsigned int mapIdx = ledY * width + ledX;

                    if(mapIdx >= map.size())
                    {
                        continue;
                    }

                    /*
                     * The same corrections of the map as the former DeviceView widget (right arrow, tab)
                     */
                    if((mapIdx + 1) < map.size() && name(map[mapIdx + 1]) == LenovoLegionDaemon::KEY_EN_RIGHT_ARROW && map[mapIdx] == NO_LED)
                    {
                        std::swap(map[mapIdx],map[mapIdx + 1]);
                    }

                    if((mapIdx + 2) < map.size() && name(map[mapIdx]) == LenovoLegionDaemon::KEY_EN_TAB && map[mapIdx + 2] == NO_LED)
                    {
                        std::swap(map[mapIdx + 1],map[mapIdx + 2]);
                    }

                    if(map[mapIdx] == NO_LED)
                    {
                        continue;
                    }

                    const unsigned int colorIdx = map[mapIdx] + zone.start_idx();

                    if(colorIdx >= ledPos.size())
                    {
                        continue;
                    }

                    Rect& pos = ledPos[colorIdx];
                    pos = { zonePos[zoneIdx].x + ledX + PAD_LED, currentY + ledY + PAD_LED, 1.0f - 2.0f * PAD_LED, 1.0f - 2.0f * PAD_LED };

                    const std::string key = name(colorIdx);

                    /*
                     * Large keys fill the empty cells: left (Tab, Caps Lock, Shift, Backspace, Num 0, ANSI Enter),
                     * up / down (Num Enter, Num +, ISO Enter), wide (Space), keys without a known code in both directions
                     */
                    if(key == LenovoLegionDaemon::KEY_EN_TAB || key == LenovoLegionDaemon::KEY_EN_CAPS_LOCK || key == LenovoLegionDaemon::KEY_EN_LEFT_SHIFT ||
                       key == LenovoLegionDaemon::KEY_EN_RIGHT_SHIFT || key == LenovoLegionDaemon::KEY_EN_BACKSPACE || key == LenovoLegionDaemon::KEY_EN_NUMPAD_0 ||
                       key == LenovoLegionDaemon::KEY_EN_ANSI_ENTER)
                    {
                        if(ledX > 0 && mapName(mapIdx - 1) == key)
                        {
                            pos.x -= 1.0f;
                            pos.w += 1.0f;
                        }
                        else if(ledX < width - 1 && (mapName(mapIdx + 1) == key || (mapIdx + 1 < map.size() && map[mapIdx + 1] == NO_LED)))
                        {
                            pos.w += 1.0f;
                        }
                    }

                    if(key == LenovoLegionDaemon::KEY_EN_NUMPAD_ENTER || key == LenovoLegionDaemon::KEY_EN_NUMPAD_PLUS)
                    {
                        if(ledY > 0 && mapName(mapIdx - width) == key)
                        {
                            pos.y -= 1.0f;
                            pos.h += 1.0f;
                        }
                        else if(ledY < height - 1 && mapName(mapIdx + width) == key)
                        {
                            pos.h += 1.0f;
                        }
                    }

                    if(key == LenovoLegionDaemon::KEY_EN_ISO_ENTER && ledY > 0 && mapName(mapIdx - width) == key)
                    {
                        pos.y -= 1.0f;
                        pos.h += 1.0f;
                    }

                    const bool unknownCode = !LenovoLegionDaemon::KeyCodesToName.contains(static_cast<int>(leds.at(static_cast<int>(colorIdx)).value()));

                    if(key == LenovoLegionDaemon::KEY_EN_SPACE || unknownCode)
                    {
                        // Space: not into the first cell of the row (the same as the former DeviceView widget)
                        const unsigned int rowStart = ledY * width + (key == LenovoLegionDaemon::KEY_EN_SPACE && !unknownCode ? 1 : 0);

                        for(unsigned int idx = mapIdx; idx > rowStart && mapName(idx - 1) == key; --idx)
                        {
                            pos.x -= 1.0f;
                            pos.w += 1.0f;
                        }

                        for(unsigned int idx = mapIdx + 1; idx < (ledY + 1) * width && mapName(idx) == key; ++idx)
                        {
                            pos.w += 1.0f;
                        }
                    }

                    if(unknownCode)
                    {
                        for(unsigned int idx = mapIdx; idx >= width && mapName(idx - width) == key; idx -= width)
                        {
                            pos.y -= 1.0f;
                            pos.h += 1.0f;
                        }

                        for(unsigned int idx = mapIdx + width; idx < map.size() && mapName(idx) == key; idx += width)
                        {
                            pos.h += 1.0f;
                        }
                    }
                }
            }

            currentY += height;
        }
        else
        {
            /*
             * Single / linear zones in rows of MAX_COLS LEDs, LEDs with the same name merged
             */
            const unsigned int count = zone.leds_count();

            for(unsigned int ledIdx = 0; ledIdx < count; ++ledIdx)
            {
                const unsigned int posIdx = zone.start_idx() + ledIdx;

                if(posIdx >= ledPos.size())
                {
                    continue;
                }

                Rect& pos = ledPos[posIdx];
                pos = { zonePos[zoneIdx].x + (ledIdx % MAX_COLS) + PAD_LED, currentY + (ledIdx / MAX_COLS) + PAD_LED, 1.0f - 2.0f * PAD_LED, 1.0f - 2.0f * PAD_LED };

                for(unsigned int i = 1; ledIdx + i < count && name(posIdx) == name(posIdx + i); ++i)
                {
                    pos.w += 1.0f;
                }

                for(unsigned int i = 1; i <= ledIdx && name(posIdx) == name(posIdx - i); ++i)
                {
                    pos.x -= 1.0f;
                    pos.w += 1.0f;
                }
            }

            currentY += (count / MAX_COLS) + ((count % MAX_COLS) > 0);
        }
    }

    /*
     * One key per group of LEDs with the same name whose rectangles touch (large keys of Spectrum keyboards are several
     * LEDs with the same name), the key covers the union of the rectangles and has all their LED indexes
     */
    struct Key
    {
        Rect             rect;
        std::string      name;
        std::vector<int> indexes;
    };

    std::vector<Key> groups;

    auto touch = [](const Rect& a,const Rect& b) {
        constexpr float GAP = 2.0f * PAD_LED + 0.01f;
        return a.x <= b.x + b.w + GAP && b.x <= a.x + a.w + GAP && a.y <= b.y + b.h + GAP && b.y <= a.y + a.h + GAP;
    };

    for(size_t ledIdx = 0; ledIdx < ledPos.size(); ++ledIdx)
    {
        const Rect& pos = ledPos[ledIdx];

        if(pos.w <= 0 || pos.h <= 0)
        {
            continue;
        }

        const std::string ledName = name(static_cast<unsigned int>(ledIdx));
        Key* group = nullptr;

        for(auto& key : groups)
        {
            if(!ledName.empty() && key.name == ledName && touch(key.rect,pos))
            {
                group = &key;
                break;
            }
        }

        if(group == nullptr)
        {
            groups.push_back({ pos, ledName, { static_cast<int>(ledIdx) } });
            continue;
        }

        const float right  = std::max(group->rect.x + group->rect.w,pos.x + pos.w);
        const float bottom = std::max(group->rect.y + group->rect.h,pos.y + pos.h);
        group->rect.x = std::min(group->rect.x,pos.x);
        group->rect.y = std::min(group->rect.y,pos.y);
        group->rect.w = right - group->rect.x;
        group->rect.h = bottom - group->rect.y;
        group->indexes.push_back(static_cast<int>(ledIdx));
    }

    /*
     * Holes of the matrix (cells without a LED) next to large keys are filled by them (Tab, Caps Lock, Shift, Enter,
     * Backspace, \, Space, Num 0 are wide on the keyboards, the matrix has one cell per LED). Other holes are real gaps
     * of the keyboard (between Copilot and the arrow up, left of the arrows, between the main block and the numpad)
     * and stay. A large key at the start / end of a row reaches the edge of the keyboard. Keys over more rows are not changed.
     */
    {
        static const std::set<std::string> LARGE_KEYS = {
            LenovoLegionDaemon::KEY_EN_TAB,         LenovoLegionDaemon::KEY_EN_CAPS_LOCK,  LenovoLegionDaemon::KEY_EN_LEFT_SHIFT,
            LenovoLegionDaemon::KEY_EN_RIGHT_SHIFT, LenovoLegionDaemon::KEY_EN_BACKSPACE,  LenovoLegionDaemon::KEY_EN_ANSI_ENTER,
            LenovoLegionDaemon::KEY_EN_ISO_ENTER,   LenovoLegionDaemon::KEY_EN_SPACE,      LenovoLegionDaemon::KEY_EN_NUMPAD_0,
            LenovoLegionDaemon::KEY_EN_ANSI_BACK_SLASH, LenovoLegionDaemon::KEY_EN_BACK_SLASH
        };

        auto large = [](const Key* key) { return LARGE_KEYS.contains(key->name); };

        constexpr float HOLE     = 2.0f * PAD_LED + 0.05f;
        // A large key fills at most one missing cell, wider holes are gaps of the keyboard (Num 0 next to the arrow up)
        constexpr float MAX_FILL = 1.0f + HOLE;

        float left = 0, right = 0;
        bool  first = true;

        for(const auto& key : groups)
        {
            left  = first ? key.rect.x : std::min(left,key.rect.x);
            right = first ? key.rect.x + key.rect.w : std::max(right,key.rect.x + key.rect.w);
            first = false;
        }

        std::map<int,std::vector<Key*>> rows;

        for(auto& key : groups)
        {
            if(key.rect.h < 1.5f)
            {
                rows[static_cast<int>(std::floor(key.rect.y + key.rect.h / 2.0f))].push_back(&key);
            }
        }

        for(auto& [row,rowKeys] : rows)
        {
            std::sort(rowKeys.begin(),rowKeys.end(),[](const Key* a,const Key* b) { return a->rect.x < b->rect.x; });

            for(size_t i = 0; i + 1 < rowKeys.size(); ++i)
            {
                Rect& a = rowKeys[i]->rect;
                Rect& b = rowKeys[i + 1]->rect;
                const float hole = b.x - (a.x + a.w);

                if(hole <= HOLE || hole > MAX_FILL || (!large(rowKeys[i]) && !large(rowKeys[i + 1])))
                {
                    continue;
                }

                // The large key grows, both large: the wider one, then the left one
                if(large(rowKeys[i + 1]) && (!large(rowKeys[i]) || b.w > a.w))
                {
                    b.w += hole - 2.0f * PAD_LED;
                    b.x  = a.x + a.w + 2.0f * PAD_LED;
                }
                else
                {
                    a.w += hole - 2.0f * PAD_LED;
                }
            }

            if(!rowKeys.empty())
            {
                Rect& firstKey = rowKeys.front()->rect;
                Rect& lastKey  = rowKeys.back()->rect;

                if(large(rowKeys.front()) && firstKey.x - left > HOLE && firstKey.x - left <= MAX_FILL)
                {
                    firstKey.w += firstKey.x - left;
                    firstKey.x  = left;
                }

                if(large(rowKeys.back()) && right - (lastKey.x + lastKey.w) > HOLE && right - (lastKey.x + lastKey.w) <= MAX_FILL)
                {
                    lastKey.w = right - lastKey.x;
                }
            }
        }
    }

    /*
     * Relative to the bounding box of the keys (no empty space of the zone labels), the width is 1
     */
    float minX = 0, minY = 0, maxX = 0, maxY = 0;

    for(size_t i = 0; i < groups.size(); ++i)
    {
        const Rect& r = groups[i].rect;
        minX = i == 0 ? r.x : std::min(minX,r.x);
        minY = i == 0 ? r.y : std::min(minY,r.y);
        maxX = i == 0 ? r.x + r.w : std::max(maxX,r.x + r.w);
        maxY = i == 0 ? r.y + r.h : std::max(maxY,r.y + r.h);
    }

    const float atom = maxX > minX ? 1.0f / (maxX - minX) : 0.0f;

    m_keys.clear();
    m_zones.clear();

    for(const auto& key : groups)
    {
        const auto label = led_label_lookup.find(key.name);
        QVariantList indexes;

        for(int index : key.indexes)
        {
            indexes.append(index);
        }

        m_keys.append(QVariantMap{
            {"index",   key.indexes.front()},
            {"indexes", indexes},
            {"x",       (key.rect.x - minX) * atom},
            {"y",       (key.rect.y - minY) * atom},
            {"w",       key.rect.w * atom},
            {"h",       key.rect.h * atom},
            {"label",   label != led_label_lookup.end() ? label->second.label_utf8 : QString()},
            {"name",    QString::fromStdString(key.name)},
            {"kind",    "key"},
            {"view",    "top"}
        });
    }

    for(int zoneIdx = 0; zoneIdx < zones.size(); ++zoneIdx)
    {
        const Rect& pos = zonePos[zoneIdx];

        m_zones.append(QVariantMap{
            {"name", QString::fromStdString(std::string(zones.at(zoneIdx).name()))},
            {"x",    (pos.x - minX) * atom},
            {"y",    (pos.y - minY) * atom},
            {"w",    pos.w * atom},
            {"h",    pos.h * atom}
        });
    }

    m_heightRatio = (maxY - minY) * atom;
    m_keyLeds.clear();

    for(int led = 0; led < leds.size(); ++led)
    {
        if(LenovoLegionDaemon::KeyCodesToName.contains(static_cast<int>(leds.at(led).value())))
        {
            m_keyLeds.append(led);
        }
    }

    m_views       = { QVariantMap{ {"name", "top"}, {"title", "Keyboard"}, {"hint", QString()}, {"heightRatio", m_heightRatio},
                                   {"deck", QVariantMap{ {"x", 0.0}, {"y", 0.0}, {"w", 1.0}, {"h", m_heightRatio} }} } };

    emit layoutChanged();
}

bool KeyboardLayout::updateByTemplate(const legion::messages::RGBControllerResponse& data)
{
    const auto& leds  = data.leds();
    const auto& zones = data.zones();

    /*
     * LEDs of the keys by the code, the same code more times (large keys) is one key
     */
    const std::vector<TemplateKey> keyTemplate = legionTemplate();

    std::map<int,size_t> slotOfCode;
    std::vector<int>     slotKey;               // template key with the rectangle of the slot (0x4E shares Left Shift)

    for(size_t i = 0; i < keyTemplate.size(); ++i)
    {
        size_t slot = slotKey.size();

        for(size_t j = 0; j < slotKey.size(); ++j)
        {
            const Rect& a = keyTemplate[static_cast<size_t>(slotKey[j])].rect;
            const Rect& b = keyTemplate[i].rect;

            if(a.x == b.x && a.y == b.y && a.w == b.w && a.h == b.h)
            {
                slot = j;
                break;
            }
        }

        if(slot == slotKey.size())
        {
            slotKey.push_back(static_cast<int>(i));
        }

        slotOfCode[keyTemplate[i].code] = slot;
    }

    std::vector<std::vector<int>> slotLeds(slotKey.size());
    int found = 0;

    for(int led = 0; led < leds.size(); ++led)
    {
        const auto slot = slotOfCode.find(static_cast<int>(leds.at(led).value()));

        if(slot != slotOfCode.end())
        {
            if(slotLeds[slot->second].empty())
            {
                ++found;
            }

            slotLeds[slot->second].push_back(led);
        }
    }

    if(found < MIN_TEMPLATE_KEYS)
    {
        return false;
    }

    /*
     * Cells of the other LEDs in the matrix (light bars), logo
     */
    struct Light
    {
        int              code;
        std::vector<int> indexes;
        int              colMin = 1 << 30, colMax = -1, rowMin = 1 << 30, rowMax = -1;
    };

    std::map<int,Light> lights;                 // by code
    std::vector<int>    logo;
    int matrixW = 1, matrixH = 1;

    for(int zoneIdx = 0; zoneIdx < zones.size(); ++zoneIdx)
    {
        const auto& zone = zones.at(zoneIdx);

        if(zone.type() != legion::messages::RGBController::ZONE_TYPE_MATRIX || zone.matrix_map().map_size() == 0)
        {
            continue;
        }

        const int width  = static_cast<int>(zone.matrix_map().width());
        const int height = static_cast<int>(zone.matrix_map().height());
        matrixW = std::max(matrixW,width);
        matrixH = std::max(matrixH,height);

        for(int cell = 0; cell < zone.matrix_map().map_size() && cell < width * height; ++cell)
        {
            const unsigned int value = zone.matrix_map().map(cell);

            if(value == NO_LED)
            {
                continue;
            }

            const int led = static_cast<int>(value + zone.start_idx());

            if(led >= leds.size())
            {
                continue;
            }

            const int code = static_cast<int>(leds.at(led).value());

            if(slotOfCode.contains(code) || code == LOGO_CODE)
            {
                continue;
            }

            Light& light = lights[code];
            light.code   = code;

            if(std::find(light.indexes.begin(),light.indexes.end(),led) == light.indexes.end())
            {
                light.indexes.push_back(led);
            }

            light.colMin = std::min(light.colMin,cell % width);
            light.colMax = std::max(light.colMax,cell % width);
            light.rowMin = std::min(light.rowMin,cell / width);
            light.rowMax = std::max(light.rowMax,cell / width);
        }
    }

    std::vector<int> others;                    // LEDs without a place (not a key, not in the matrix)

    for(int led = 0; led < leds.size(); ++led)
    {
        const int code = static_cast<int>(leds.at(led).value());

        if(code == LOGO_CODE)
        {
            logo.push_back(led);
        }
        else if(!slotOfCode.contains(code) && !lights.contains(code))
        {
            others.push_back(led);
        }
    }

    /*
     * Three views of the notebook with the same scale (width of the chassis):
     *   top:   open lid with the display, the keys on the deck, unknown LEDs
     *   front: the display, the front edge of the chassis with the front light bar
     *   back:  back of the lid with the logo, rear edge of the chassis with the vents and the rear light bar
     * Rear light bar (top row of the matrix): segments in the order of the columns along a line around the vents.
     * Front light bar (around the matrix): one strip in the order around the matrix (left column down, bottom row,
     * right column up), the width of a segment by its cells.
     */
    struct Item
    {
        Rect             rect;
        std::vector<int> indexes;
        QString          kind;
        QString          label;
        QString          view;
        std::vector<std::pair<float,float>> path;   // light along a line (rear bar), points in the view
    };

    std::vector<Item> items;

    for(size_t slot = 0; slot < slotKey.size(); ++slot)
    {
        if(!slotLeds[slot].empty())
        {
            const TemplateKey& key = keyTemplate[static_cast<size_t>(slotKey[slot])];
            items.push_back({ key.rect, slotLeds[slot], "key", key.label, "top", {} });
        }
    }

    constexpr float SIDE        = 0.8f;
    constexpr float CHASSIS_X   = -1.2f;
    constexpr float CHASSIS_W   = KB_WIDTH + 2.4f;

    // Top view
    constexpr float TOP_LID_Y   = -6.0f;
    constexpr float TOP_BODY_Y  = -1.3f;
    constexpr float TOP_BOTTOM  = KB_HEIGHT + 0.9f;

    // Front view
    constexpr float FRONT_LID_H  = 8.0f;
    constexpr float FRONT_BODY_Y = FRONT_LID_H + 0.3f;
    constexpr float FRONT_BODY_H = 1.5f;
    constexpr float FRONT_LIGHT  = 0.32f;

    // Back view
    // Lid and the rear edge of the chassis with the same sizes as in the front view
    constexpr float BACK_LID_H   = FRONT_LID_H;
    constexpr float BACK_BODY_Y  = FRONT_BODY_Y;
    constexpr float BACK_BODY_H  = FRONT_BODY_H;

    /*
     * Rear light bar around the vents (photo of the back): two rows of LEDs in the top row of the matrix (interleaved
     * columns). The upper row (codes below REAR_BOTTOM_CODE) goes up the left outer edge, over the top of the left vents,
     * down to the middle, over the top of the right vents and down the right outer edge. The lower row is under the left
     * vents (first half by columns) and under the right vents (second half), up the inner edges to the middle.
     */
    constexpr int   REAR_BOTTOM_CODE = 1011;
    constexpr float VENT_TOP     = BACK_BODY_Y + 0.22f;
    constexpr float VENT_BOTTOM  = BACK_BODY_Y + BACK_BODY_H - 0.22f;
    constexpr float REAR_MIDDLE  = (VENT_TOP + VENT_BOTTOM) / 2.0f;   // middle line in the vertical middle
    constexpr float LEFT_END     = 7.2f;
    constexpr float RIGHT_START  = KB_WIDTH - LEFT_END;
    constexpr float CHAMFER      = 0.5f;
    constexpr float CORNER       = 0.2f;
    constexpr float OUTER_LEFT   = -0.3f;
    constexpr float OUTER_RIGHT  = KB_WIDTH + 0.3f;
    constexpr float INNER_LEFT   = LEFT_END + CHAMFER;
    constexpr float INNER_RIGHT  = RIGHT_START - CHAMFER;

    using Path = std::vector<std::pair<float,float>>;

    const Path upperPath = {
        { OUTER_LEFT, VENT_BOTTOM - CORNER },   { OUTER_LEFT, VENT_TOP + CORNER },   { OUTER_LEFT + CORNER, VENT_TOP },
        { LEFT_END, VENT_TOP },                 { INNER_LEFT, REAR_MIDDLE },         { INNER_RIGHT, REAR_MIDDLE },
        { RIGHT_START, VENT_TOP },              { OUTER_RIGHT - CORNER, VENT_TOP },  { OUTER_RIGHT, VENT_TOP + CORNER },
        { OUTER_RIGHT, VENT_BOTTOM - CORNER }
    };

    const Path lowerLeftPath = {
        { OUTER_LEFT, VENT_BOTTOM - CORNER },   { OUTER_LEFT + CORNER, VENT_BOTTOM },  { INNER_LEFT - CORNER, VENT_BOTTOM },
        { INNER_LEFT, VENT_BOTTOM - CORNER },   { INNER_LEFT, REAR_MIDDLE + 0.1f }
    };

    const Path lowerRightPath = {
        { INNER_RIGHT, REAR_MIDDLE + 0.1f },   { INNER_RIGHT, VENT_BOTTOM - CORNER }, { INNER_RIGHT + CORNER, VENT_BOTTOM },
        { OUTER_RIGHT - CORNER, VENT_BOTTOM },  { OUTER_RIGHT, VENT_BOTTOM - CORNER }
    };

    auto pathLength = [](const Path& path) {
        float length = 0;

        for(size_t i = 0; i + 1 < path.size(); ++i)
        {
            length += std::hypot(path[i + 1].first - path[i].first,path[i + 1].second - path[i].second);
        }

        return length;
    };

    // Part of the path between the distances from its start
    auto pathPart = [](const Path& path,float from,float to) {
        Path  part;
        float distance = 0;

        for(size_t i = 0; i + 1 < path.size(); ++i)
        {
            const auto [x0,y0] = path[i];
            const auto [x1,y1] = path[i + 1];
            const float length = std::hypot(x1 - x0,y1 - y0);
            const float a      = std::clamp((from - distance) / length,0.0f,1.0f);
            const float b      = std::clamp((to - distance) / length,0.0f,1.0f);

            if(b > a)
            {
                if(part.empty())
                {
                    part.push_back({ x0 + (x1 - x0) * a, y0 + (y1 - y0) * a });
                }

                part.push_back({ x0 + (x1 - x0) * b, y0 + (y1 - y0) * b });
            }

            distance += length;
        }

        return part;
    };

    std::vector<const Light*> front;

    for(const auto& [code,light] : lights)
    {
        if(light.rowMax != 0)
        {
            front.push_back(&light);
        }
    }

    // Position around the matrix: left column (by row), bottom row (by column), right column (by row up)
    auto aroundMatrix = [matrixW,matrixH](const Light* light) {
        if(light->colMax == 0)
        {
            return light->rowMin;
        }

        if(light->colMin == matrixW - 1)
        {
            return 2 * matrixH + matrixW - light->rowMin;
        }

        return matrixH + light->colMin;
    };

    std::sort(front.begin(),front.end(),[&aroundMatrix](const Light* a,const Light* b) { return aroundMatrix(a) < aroundMatrix(b); });

    auto cells = [](const Light* light) { return static_cast<float>((light->colMax - light->colMin + 1) * (light->rowMax - light->rowMin + 1)); };

    float frontCells = 0;

    for(const Light* light : front)
    {
        frontCells += cells(light);
    }

    // Rear LEDs of the upper and the lower row in the order of the columns, the lower row split into left and right
    std::vector<const Light*> upper, lower;

    for(const auto& [code,light] : lights)
    {
        if(light.rowMax == 0)
        {
            (code >= REAR_BOTTOM_CODE ? lower : upper).push_back(&light);
        }
    }

    auto byColumn = [](const Light* a,const Light* b) { return a->colMin < b->colMin; };
    std::sort(upper.begin(),upper.end(),byColumn);
    std::sort(lower.begin(),lower.end(),byColumn);

    const std::vector<const Light*> lowerLeft(lower.begin(),lower.begin() + static_cast<long>(lower.size() / 2));
    const std::vector<const Light*> lowerRight(lower.begin() + static_cast<long>(lower.size() / 2),lower.end());

    // Segments with the same length along the path, a small gap between them
    auto alongPath = [&](const std::vector<const Light*>& rearLights,const Path& path) {
        const float length = pathLength(path);
        const float gap    = 0.04f;

        for(size_t i = 0; i < rearLights.size(); ++i)
        {
            const float step = length / static_cast<float>(rearLights.size());
            const Path  part = pathPart(path,step * static_cast<float>(i) + gap,step * static_cast<float>(i + 1) - gap);

            float minX = part.front().first, minY = part.front().second, maxX = minX, maxY = minY;

            for(const auto& [x,y] : part)
            {
                minX = std::min(minX,x); maxX = std::max(maxX,x);
                minY = std::min(minY,y); maxY = std::max(maxY,y);
            }

            const float half = LIGHT_THICKNESS / 2.0f;
            items.push_back({ { minX - half, minY - half, maxX - minX + LIGHT_THICKNESS, maxY - minY + LIGHT_THICKNESS }, rearLights[i]->indexes, "rear", {}, "back", part });
        }
    };

    alongPath(upper,upperPath);
    alongPath(lowerLeft,lowerLeftPath);
    alongPath(lowerRight,lowerRightPath);

    float frontX = -SIDE;

    for(const Light* light : front)
    {
        const float width = (KB_WIDTH + 2.0f * SIDE) * cells(light) / frontCells;

        items.push_back({ { frontX, FRONT_BODY_Y + FRONT_BODY_H - FRONT_LIGHT - 0.2f, width, FRONT_LIGHT }, light->indexes, "front", {}, "front", {} });
        frontX += width;
    }

    if(!logo.empty())
    {
        items.push_back({ { KB_WIDTH / 2.0f - 2.75f, BACK_LID_H * 0.42f, 5.5f, 1.3f }, logo, "logo", "LEGION", "back", {} });
    }

    float topBottom = TOP_BOTTOM;

    for(size_t i = 0; i < others.size(); ++i)
    {
        constexpr float PITCH = KB_WIDTH / MAX_COLS;
        const Rect rect = { (i % MAX_COLS) * PITCH, TOP_BOTTOM + 0.3f + (i / MAX_COLS) * 0.9f, PITCH, 0.8f };

        items.push_back({ rect, { others[i] }, "other", {}, "top", {} });
        topBottom = std::max(topBottom,rect.y + rect.h);
    }

    /*
     * Relative to the chassis width (the same in all views), y from the top of the view
     */
    const float atom = 1.0f / CHASSIS_W;

    auto relative = [atom](const Rect& r,float top) {
        return QVariantMap{ {"x", (r.x - CHASSIS_X) * atom}, {"y", (r.y - top) * atom}, {"w", r.w * atom}, {"h", r.h * atom} };
    };

    auto viewTop = [&](const QString& view) {
        return view == "top" ? TOP_LID_Y : 0.0f;
    };

    m_keys.clear();
    m_zones.clear();

    for(const auto& item : items)
    {
        const std::string name  = std::string(leds.at(item.indexes.front()).name());
        const auto        label = led_label_lookup.find(name);
        QVariantList      indexes;

        for(int index : item.indexes)
        {
            indexes.append(index);
        }

        QVariantMap key = relative(item.rect,viewTop(item.view));
        key.insert("index",   item.indexes.front());
        key.insert("indexes", indexes);
        key.insert("kind",    item.kind);
        key.insert("view",    item.view);

        if(!item.path.empty())
        {
            QVariantList path;

            for(const auto& [x,y] : item.path)
            {
                path.append(QVariantMap{ {"x", (x - item.rect.x) * atom}, {"y", (y - item.rect.y) * atom} });
            }

            key.insert("path", path);
            key.insert("thickness", LIGHT_THICKNESS * atom);
        }
        key.insert("name",    item.kind == "rear"  ? QString("Rear light ").append(QString::fromStdString(name))
                            : item.kind == "front" ? QString("Front light ").append(QString::fromStdString(name))
                            : item.kind == "logo"  ? QString("Logo")
                            : QString::fromStdString(name));
        key.insert("label",   !item.label.isEmpty() ? item.label : label != led_label_lookup.end() ? label->second.label_utf8 : QString());
        m_keys.append(key);
    }

    // Vents of the rear: four openings inside the left and the right outline
    QVariantList vents;
    {
        const float ventY = VENT_TOP + 0.3f;
        const float ventH = VENT_BOTTOM - 0.3f - ventY;
        const float ventW = (INNER_LEFT - 0.4f - (OUTER_LEFT + 0.4f)) / 4.0f;

        for(int i = 0; i < 4; ++i)
        {
            vents.append(relative({ OUTER_LEFT + 0.4f + i * ventW + 0.08f, ventY, ventW - 0.16f, ventH },0));
            vents.append(relative({ INNER_RIGHT + 0.4f + i * ventW + 0.08f, ventY, ventW - 0.16f, ventH },0));
        }

    }

    auto view = [&](const QString& name,const QString& title,const QString& hint,float top,float bottom) {
        return QVariantMap{ {"name", name}, {"title", title}, {"hint", hint}, {"heightRatio", (bottom - top) * atom} };
    };

    QVariantMap top = view("top","Keyboard","Keys of the keyboard, seen from above",TOP_LID_Y,topBottom);
    top.insert("lid",   relative({ CHASSIS_X, TOP_LID_Y, CHASSIS_W, TOP_BODY_Y - 0.25f - TOP_LID_Y },TOP_LID_Y));
    top.insert("body",  relative({ CHASSIS_X, TOP_BODY_Y, CHASSIS_W, TOP_BOTTOM - TOP_BODY_Y },TOP_LID_Y));
    top.insert("deck",  relative({ 0, 0, KB_WIDTH, KB_HEIGHT },TOP_LID_Y));

    QVariantMap frontView = view("front","Front","Light bar along the front edge of the chassis",0,FRONT_BODY_Y + FRONT_BODY_H);
    frontView.insert("lid",  relative({ CHASSIS_X + 0.6f, 0, CHASSIS_W - 1.2f, FRONT_LID_H },0));
    frontView.insert("body", relative({ CHASSIS_X, FRONT_BODY_Y, CHASSIS_W, FRONT_BODY_H },0));

    QVariantMap back = view("back","Back","Logo on the lid and the light bar around the rear vents",0,BACK_BODY_Y + BACK_BODY_H);
    back.insert("lid",   relative({ CHASSIS_X + 0.6f, 0, CHASSIS_W - 1.2f, BACK_LID_H },0));
    back.insert("body",  relative({ CHASSIS_X, BACK_BODY_Y, CHASSIS_W, BACK_BODY_H },0));
    back.insert("vents", vents);

    m_keyLeds.clear();

    for(const auto& slot : slotLeds)
    {
        for(int led : slot)
        {
            m_keyLeds.append(led);
        }
    }

    m_views = { top };

    if(!front.empty())
    {
        m_views.append(frontView);
    }

    if(!upper.empty() || !lower.empty() || !logo.empty())
    {
        m_views.append(back);
    }

    m_heightRatio = top.value("heightRatio").toReal();

    return true;
}

}
