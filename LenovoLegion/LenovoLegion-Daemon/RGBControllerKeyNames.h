// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright Jaroslav Bolek 2025
 *
 * Author(s):
 *   Jaroslav Bolek <jaroslav.bolek@gmail.com>
 */

#pragma once

#include <map>
#include <string>

namespace LenovoLegionDaemon {

#define NA 0xFFFFFFFF

constexpr const char* KEY_EN_UNUSED             = "";
constexpr const char* ZONE_EN_KEYBOARD          = "Keyboard";

constexpr const char* KEY_EN_ESCAPE             = "Key: Escape";
constexpr const char* KEY_EN_F1                 = "Key: F1";
constexpr const char* KEY_EN_F2                 = "Key: F2";
constexpr const char* KEY_EN_F3                 = "Key: F3";
constexpr const char* KEY_EN_F4                 = "Key: F4";
constexpr const char* KEY_EN_F5                 = "Key: F5";
constexpr const char* KEY_EN_F6                 = "Key: F6";
constexpr const char* KEY_EN_F7                 = "Key: F7";
constexpr const char* KEY_EN_F8                 = "Key: F8";
constexpr const char* KEY_EN_F9                 = "Key: F9";
constexpr const char* KEY_EN_F10                = "Key: F10";
constexpr const char* KEY_EN_F11                = "Key: F11";
constexpr const char* KEY_EN_F12                = "Key: F12";
constexpr const char* KEY_EN_PRINT_SCREEN       = "Key: Print Screen";
constexpr const char* KEY_EN_SCROLL_LOCK        = "Key: Scroll Lock";
constexpr const char* KEY_EN_PAUSE_BREAK        = "Key: Pause/Break";
constexpr const char* KEY_EN_POWER              = "Key: Power";

constexpr const char* KEY_EN_BACK_TICK          = "Key: `";
constexpr const char* KEY_EN_1                  = "Key: 1";
constexpr const char* KEY_EN_2                  = "Key: 2";
constexpr const char* KEY_EN_3                  = "Key: 3";
constexpr const char* KEY_EN_4                  = "Key: 4";
constexpr const char* KEY_EN_5                  = "Key: 5";
constexpr const char* KEY_EN_6                  = "Key: 6";
constexpr const char* KEY_EN_7                  = "Key: 7";
constexpr const char* KEY_EN_8                  = "Key: 8";
constexpr const char* KEY_EN_9                  = "Key: 9";
constexpr const char* KEY_EN_0                  = "Key: 0";
constexpr const char* KEY_EN_MINUS              = "Key: -";
constexpr const char* KEY_EN_PLUS               = "Key: +";
constexpr const char* KEY_EN_EQUALS             = "Key: =";
constexpr const char* KEY_EN_BACKSPACE          = "Key: Backspace";
constexpr const char* KEY_EN_INSERT             = "Key: Insert";
constexpr const char* KEY_EN_HOME               = "Key: Home";
constexpr const char* KEY_EN_PAGE_UP            = "Key: Page Up";

constexpr const char* KEY_EN_TAB                = "Key: Tab";
constexpr const char* KEY_EN_Q                  = "Key: Q";
constexpr const char* KEY_EN_W                  = "Key: W";
constexpr const char* KEY_EN_E                  = "Key: E";
constexpr const char* KEY_EN_R                  = "Key: R";
constexpr const char* KEY_EN_T                  = "Key: T";
constexpr const char* KEY_EN_Y                  = "Key: Y";
constexpr const char* KEY_EN_U                  = "Key: U";
constexpr const char* KEY_EN_I                  = "Key: I";
constexpr const char* KEY_EN_O                  = "Key: O";
constexpr const char* KEY_EN_P                  = "Key: P";
constexpr const char* KEY_EN_LEFT_BRACKET       = "Key: [";
constexpr const char* KEY_EN_RIGHT_BRACKET      = "Key: ]";
constexpr const char* KEY_EN_BACK_SLASH         = "Key: \\";
constexpr const char* KEY_EN_ANSI_BACK_SLASH    = "Key: \\ (ANSI)";
constexpr const char* KEY_EN_DELETE             = "Key: Delete";
constexpr const char* KEY_EN_END                = "Key: End";
constexpr const char* KEY_EN_PAGE_DOWN          = "Key: Page Down";

constexpr const char* KEY_EN_CAPS_LOCK          = "Key: Caps Lock";
constexpr const char* KEY_EN_A                  = "Key: A";
constexpr const char* KEY_EN_S                  = "Key: S";
constexpr const char* KEY_EN_D                  = "Key: D";
constexpr const char* KEY_EN_F                  = "Key: F";
constexpr const char* KEY_EN_G                  = "Key: G";
constexpr const char* KEY_EN_H                  = "Key: H";
constexpr const char* KEY_EN_J                  = "Key: J";
constexpr const char* KEY_EN_K                  = "Key: K";
constexpr const char* KEY_EN_L                  = "Key: L";
constexpr const char* KEY_EN_SEMICOLON          = "Key: ;";
constexpr const char* KEY_EN_QUOTE              = "Key: '";
constexpr const char* KEY_EN_POUND              = "Key: #";
constexpr const char* KEY_EN_ANSI_ENTER         = "Key: Enter";
constexpr const char* KEY_EN_ISO_ENTER          = "Key: Enter (ISO)";

constexpr const char* KEY_EN_LEFT_SHIFT         = "Key: Left Shift";
constexpr const char* KEY_EN_ISO_BACK_SLASH     = "Key: \\ (ISO)";
constexpr const char* KEY_EN_Z                  = "Key: Z";
constexpr const char* KEY_EN_X                  = "Key: X";
constexpr const char* KEY_EN_C                  = "Key: C";
constexpr const char* KEY_EN_V                  = "Key: V";
constexpr const char* KEY_EN_B                  = "Key: B";
constexpr const char* KEY_EN_N                  = "Key: N";
constexpr const char* KEY_EN_M                  = "Key: M";
constexpr const char* KEY_EN_COMMA              = "Key: ,";
constexpr const char* KEY_EN_PERIOD             = "Key: .";
constexpr const char* KEY_EN_FORWARD_SLASH      = "Key: /";
constexpr const char* KEY_EN_RIGHT_SHIFT        = "Key: Right Shift";
constexpr const char* KEY_EN_UP_ARROW           = "Key: Up Arrow";

constexpr const char* KEY_EN_LEFT_CONTROL       = "Key: Left Control";
constexpr const char* KEY_EN_LEFT_WINDOWS       = "Key: Left Windows";
constexpr const char* KEY_EN_LEFT_FUNCTION      = "Key: Left Fn";
constexpr const char* KEY_EN_LEFT_ALT           = "Key: Left Alt";
constexpr const char* KEY_EN_SPACE              = "Key: Space";
constexpr const char* KEY_EN_RIGHT_ALT          = "Key: Right Alt";
constexpr const char* KEY_EN_RIGHT_FUNCTION     = "Key: Right Fn";
constexpr const char* KEY_EN_RIGHT_WINDOWS      = "Key: Right Windows";
constexpr const char* KEY_EN_MENU               = "Key: Menu";
constexpr const char* KEY_EN_RIGHT_CONTROL      = "Key: Right Control";
constexpr const char* KEY_EN_LEFT_ARROW         = "Key: Left Arrow";
constexpr const char* KEY_EN_DOWN_ARROW         = "Key: Down Arrow";
constexpr const char* KEY_EN_RIGHT_ARROW        = "Key: Right Arrow";

constexpr const char* KEY_EN_NUMPAD_LOCK        = "Key: Num Lock";
constexpr const char* KEY_EN_NUMPAD_DIVIDE      = "Key: Number Pad /";
constexpr const char* KEY_EN_NUMPAD_TIMES       = "Key: Number Pad *";
constexpr const char* KEY_EN_NUMPAD_MINUS       = "Key: Number Pad -";
constexpr const char* KEY_EN_NUMPAD_PLUS        = "Key: Number Pad +";
constexpr const char* KEY_EN_NUMPAD_PERIOD      = "Key: Number Pad .";
constexpr const char* KEY_EN_NUMPAD_ENTER       = "Key: Number Pad Enter";
constexpr const char* KEY_EN_NUMPAD_0           = "Key: Number Pad 0";
constexpr const char* KEY_EN_NUMPAD_1           = "Key: Number Pad 1";
constexpr const char* KEY_EN_NUMPAD_2           = "Key: Number Pad 2";
constexpr const char* KEY_EN_NUMPAD_3           = "Key: Number Pad 3";
constexpr const char* KEY_EN_NUMPAD_4           = "Key: Number Pad 4";
constexpr const char* KEY_EN_NUMPAD_5           = "Key: Number Pad 5";
constexpr const char* KEY_EN_NUMPAD_6           = "Key: Number Pad 6";
constexpr const char* KEY_EN_NUMPAD_7           = "Key: Number Pad 7";
constexpr const char* KEY_EN_NUMPAD_8           = "Key: Number Pad 8";
constexpr const char* KEY_EN_NUMPAD_9           = "Key: Number Pad 9";

constexpr const char* KEY_EN_MEDIA_PLAY_PAUSE   = "Key: Media Play/Pause";
constexpr const char* KEY_EN_MEDIA_PREVIOUS     = "Key: Media Previous";
constexpr const char* KEY_EN_MEDIA_NEXT         = "Key: Media Next";
constexpr const char* KEY_EN_MEDIA_STOP         = "Key: Media Stop";
constexpr const char* KEY_EN_MEDIA_MUTE         = "Key: Media Mute";
constexpr const char* KEY_EN_MEDIA_VOLUME_DOWN  = "Key: Media Volume -";
constexpr const char* KEY_EN_MEDIA_VOLUME_UP    = "Key: Media Volume +";

constexpr const char* KEY_JP_RO                 = "Key: _";
constexpr const char* KEY_JP_EJ                 = "Key: E/J";
constexpr const char* KEY_JP_ZENKAKU            = "Key: 半角/全角";
constexpr const char* KEY_JP_KANA               = "Key: かな";
constexpr const char* KEY_JP_HENKAN             = "Key: 変換";
constexpr const char* KEY_JP_MUHENKAN           = "Key: 無変換";
constexpr const char* KEY_JP_YEN                = "Key: ¥";
constexpr const char* KEY_JP_AT                 = "Key: @";
constexpr const char* KEY_JP_CHEVRON            = "Key: ^";
constexpr const char* KEY_JP_COLON              = "Key: :";

constexpr const char* KEY_KR_HAN                = "Key: 한/영";
constexpr const char* KEY_KR_HANJA              = "Key: 한자";

constexpr const char* KEY_NORD_AAL              = "Key: Å";
constexpr const char* KEY_NORD_A_OE             = "Key: Ä Ø";
constexpr const char* KEY_NORD_O_AE             = "Key: Ö Æ";
constexpr const char* KEY_NORD_HALF             = "Key: § ½";
constexpr const char* KEY_NORD_HYPHEN           = "Key: - _";
constexpr const char* KEY_NORD_PLUS_QUESTION    = "Key: + ?";
constexpr const char* KEY_NORD_ACUTE_GRAVE      = "Key: ´ `";
constexpr const char* KEY_NORD_DOTS_CARET       = "Key: ¨ ^";
constexpr const char* KEY_NORD_QUOTE            = "Key: ' *";
constexpr const char* KEY_NORD_ANGLE_BRACKET    = "Key: < >";

constexpr const char* KEY_DE_ESZETT             = "Key: ß";
constexpr const char* KEY_DE_DIAERESIS_A        = "Key: Ä";
constexpr const char* KEY_DE_DIAERESIS_O        = "Key: Ö";
constexpr const char* KEY_DE_DIAERESIS_U        = "Key: Ü";

constexpr const char* KEY_FR_SUPER_2            = "Key: ²";
constexpr const char* KEY_FR_AMPERSAND          = "Key: &";
constexpr const char* KEY_FR_ACUTE_E            = "Key: é";
constexpr const char* KEY_FR_DOUBLEQUOTE        = "Key: \"";
constexpr const char* KEY_FR_LEFT_PARENTHESIS   = "Key: (";
constexpr const char* KEY_FR_GRAVE_E            = "Key: è";
constexpr const char* KEY_FR_UNDERSCORE         = "Key: _";
constexpr const char* KEY_FR_CEDILLA_C          = "Key: ç";
constexpr const char* KEY_FR_GRAVE_A            = "Key: à";
constexpr const char* KEY_FR_RIGHT_PARENTHESIS  = "Key: )";
constexpr const char* KEY_FR_DOLLAR             = "Key: $";
constexpr const char* KEY_FR_GRAVE_U            = "Key: ù";
constexpr const char* KEY_FR_ASTERIX            = "Key: *";
constexpr const char* KEY_FR_EXCLAIMATION       = "Key: !";

constexpr const char* KEY_ES_OPEN_QUESTION_MARK = "Key: ¿/¡";
constexpr const char* KEY_ES_TILDE              = "Key: ´/¨";
constexpr const char* KEY_ES_ENIE               = "Key: Ñ";


inline const std::map<int,std::string> KeyCodesToName =
{
    // Row 1 - Function keys and special keys
    {0x01, KEY_EN_ESCAPE},
    {0x02, KEY_EN_F1},
    {0x03, KEY_EN_F2},
    {0x04, KEY_EN_F3},
    {0x05, KEY_EN_F4},
    {0x06, KEY_EN_F5},
    {0x07, KEY_EN_F6},
    {0x08, KEY_EN_F7},
    {0x09, KEY_EN_F8},
    {0x0A, KEY_EN_F9},
    {0x0B, KEY_EN_F10},
    {0x0C, KEY_EN_F11},
    {0x0D, KEY_EN_F12},
    {0x0E, KEY_EN_INSERT},
    {0x0F, KEY_EN_PRINT_SCREEN},
    {0x10, KEY_EN_DELETE},
    {0x11, KEY_EN_HOME},
    {0x12, KEY_EN_END},
    {0x13, KEY_EN_PAGE_UP},
    {0x14, KEY_EN_PAGE_DOWN},

    // Row 2 - Number row
    {0x16, KEY_EN_BACK_TICK},
    {0x17, KEY_EN_1},
    {0x18, KEY_EN_2},
    {0x19, KEY_EN_3},
    {0x1A, KEY_EN_4},
    {0x1B, KEY_EN_5},
    {0x1C, KEY_EN_6},
    {0x1D, KEY_EN_7},
    {0x1E, KEY_EN_8},
    {0x1F, KEY_EN_9},
    {0x20, KEY_EN_0},
    {0x21, KEY_EN_MINUS},
    {0x22, KEY_EN_EQUALS},
    {0x38, KEY_EN_BACKSPACE},
    {0x26, KEY_EN_NUMPAD_LOCK},
    {0x27, KEY_EN_NUMPAD_DIVIDE},
    {0x28, KEY_EN_NUMPAD_TIMES},
    {0x29, KEY_EN_NUMPAD_MINUS},

    // Row 3 - QWERTY row
    {0x40, KEY_EN_TAB},
    {0x42, KEY_EN_Q},
    {0x43, KEY_EN_W},
    {0x44, KEY_EN_E},
    {0x45, KEY_EN_R},
    {0x46, KEY_EN_T},
    {0x47, KEY_EN_Y},
    {0x48, KEY_EN_U},
    {0x49, KEY_EN_I},
    {0x4A, KEY_EN_O},
    {0x4B, KEY_EN_P},
    {0x4C, KEY_EN_LEFT_BRACKET},
    {0x4D, KEY_EN_RIGHT_BRACKET},
    {0x4E, KEY_EN_ISO_BACK_SLASH},
    {0x4F, KEY_EN_NUMPAD_7},
    {0x50, KEY_EN_NUMPAD_8},
    {0x51, KEY_EN_NUMPAD_9},

    // Row 4 - ASDF row
    {0x55, KEY_EN_CAPS_LOCK},
    {0x6D, KEY_EN_A},
    {0x6E, KEY_EN_S},
    {0x58, KEY_EN_D},
    {0x59, KEY_EN_F},
    {0x5A, KEY_EN_G},
    {0x71, KEY_EN_H},
    {0x72, KEY_EN_J},
    {0x5B, KEY_EN_K},
    {0x5C, KEY_EN_L},
    {0x5D, KEY_EN_SEMICOLON},
    {0x5F, KEY_EN_QUOTE},
    {0xA8, KEY_EN_ISO_BACK_SLASH},
    {0x77, KEY_EN_ISO_ENTER},
    {0x79, KEY_EN_NUMPAD_4},
    {0x7B, KEY_EN_NUMPAD_5},
    {0x7C, KEY_EN_NUMPAD_6},
    {0x68, KEY_EN_NUMPAD_PLUS},

    // Row 5 - ZXCV row
    {0x6A, KEY_EN_LEFT_SHIFT},
    {0x82, KEY_EN_Z},
    {0x83, KEY_EN_X},
    {0x6F, KEY_EN_C},
    {0x70, KEY_EN_V},
    {0x87, KEY_EN_B},
    {0x88, KEY_EN_N},
    {0x73, KEY_EN_M},
    {0x74, KEY_EN_COMMA},
    {0x75, KEY_EN_PERIOD},
    {0x76, KEY_EN_FORWARD_SLASH},
    {0x8D, KEY_EN_RIGHT_SHIFT},
    {0x8E, KEY_EN_NUMPAD_1},
    {0x90, KEY_EN_NUMPAD_2},
    {0x92, KEY_EN_NUMPAD_3},

    // Row 6 - Bottom row
    {0x7F, KEY_EN_LEFT_CONTROL},
    {0x80, KEY_EN_LEFT_FUNCTION},
    {0x96, KEY_EN_LEFT_WINDOWS},
    {0x97, KEY_EN_LEFT_ALT},
    {0x98, KEY_EN_SPACE},
    {0x9A, KEY_EN_RIGHT_ALT},
    {0x9B, KEY_EN_RIGHT_CONTROL},
    {0x9D, KEY_EN_UP_ARROW},
    {0xA3, KEY_EN_NUMPAD_0},
    {0xA5, KEY_EN_NUMPAD_PERIOD},
    {0xA7, KEY_EN_NUMPAD_ENTER},

    // Row 7 - Arrow keys
    {0x9C, KEY_EN_LEFT_ARROW},
    {0x9F, KEY_EN_DOWN_ARROW},
    {0xA1, KEY_EN_RIGHT_ARROW},
};

}
