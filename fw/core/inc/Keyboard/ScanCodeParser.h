/**
 * @file     ScanCodeParser.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     11.07.2025
 * @brief    Parser for Scan Code Set 2
 * @see      Keyboards (101- and 102-Key)
 *           http://www.mcamafia.de/pdf/ibm_hitrc11.pdf
 *           Microsoft USB HID to PS/2 Translation Table - Scan Code Set 1 and 2
 *           http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf
 */

#ifndef SCANCODEPARSER_H
#define SCANCODEPARSER_H

#include <array>
#include "UniKey.h"

class ScanCodeParser {
 public:
  enum Layout : uint8_t { NONUS_102K, US_101K };
  enum Action : uint8_t { NONE = 0, MAKE, BREAK, RESET };
  struct ParsedKey {
    Action action;
    UniKey key;
  };

  ScanCodeParser(Layout layout = NONUS_102K);
  void setLayout(Layout layout);
  void reset();

  ParsedKey parse(uint8_t code);

 private:
  enum State {
    MAKE_START,
    BREAK_F0,
    MAKE_E0,
    BREAK_E0_F0,
    MAKE_E1,
    MAKE_E1_14,
    MAKE_E1_14_77,
    BREAK_E1,
    BREAK_E1_F0,
    BREAK_E1_F0_14,
    BREAK_E1_F0_14_F0
  };

  static constexpr auto KeysLut() {
    std::array<UniKey, 256> lut{};

    lut[0x01] = UniKey::KB_F9;
    lut[0x03] = UniKey::KB_F5;
    lut[0x04] = UniKey::KB_F3;
    lut[0x05] = UniKey::KB_F1;
    lut[0x06] = UniKey::KB_F2;
    lut[0x07] = UniKey::KB_F12;
    lut[0x08] = UniKey::KB_F13;
    lut[0x09] = UniKey::KB_F10;
    lut[0x0A] = UniKey::KB_F8;
    lut[0x0B] = UniKey::KB_F6;
    lut[0x0C] = UniKey::KB_F4;
    lut[0x0D] = UniKey::KB_TAB;
    lut[0x0E] = UniKey::KB_GRAVE;
    lut[0x0F] = UniKey::KP_EQUAL;
    lut[0x10] = UniKey::KB_F14;
    lut[0x11] = UniKey::KB_LALT;
    lut[0x12] = UniKey::KB_LSHIFT;
    lut[0x13] = UniKey::KB_INT2;
    lut[0x14] = UniKey::KB_LCTRL;
    lut[0x15] = UniKey::KB_Q;
    lut[0x16] = UniKey::KB_1;
    lut[0x18] = UniKey::KB_F15;
    lut[0x1A] = UniKey::KB_Z;
    lut[0x1B] = UniKey::KB_S;
    lut[0x1C] = UniKey::KB_A;
    lut[0x1D] = UniKey::KB_W;
    lut[0x1E] = UniKey::KB_2;
    lut[0x20] = UniKey::KB_F16;
    lut[0x21] = UniKey::KB_C;
    lut[0x22] = UniKey::KB_X;
    lut[0x23] = UniKey::KB_D;
    lut[0x24] = UniKey::KB_E;
    lut[0x25] = UniKey::KB_4;
    lut[0x26] = UniKey::KB_3;
    lut[0x27] = UniKey::KB_INT6;
    lut[0x28] = UniKey::KB_F17;
    lut[0x29] = UniKey::KB_SPACE;
    lut[0x2A] = UniKey::KB_V;
    lut[0x2B] = UniKey::KB_F;
    lut[0x2C] = UniKey::KB_T;
    lut[0x2D] = UniKey::KB_R;
    lut[0x2E] = UniKey::KB_5;
    lut[0x30] = UniKey::KB_F18;
    lut[0x31] = UniKey::KB_N;
    lut[0x32] = UniKey::KB_B;
    lut[0x33] = UniKey::KB_H;
    lut[0x34] = UniKey::KB_G;
    lut[0x35] = UniKey::KB_Y;
    lut[0x36] = UniKey::KB_6;
    lut[0x38] = UniKey::KB_F19;
    lut[0x3A] = UniKey::KB_M;
    lut[0x3B] = UniKey::KB_J;
    lut[0x3C] = UniKey::KB_U;
    lut[0x3D] = UniKey::KB_7;
    lut[0x3E] = UniKey::KB_8;
    lut[0x40] = UniKey::KB_F20;
    lut[0x41] = UniKey::KB_COMMA;
    lut[0x42] = UniKey::KB_K;
    lut[0x43] = UniKey::KB_I;
    lut[0x44] = UniKey::KB_O;
    lut[0x45] = UniKey::KB_0;
    lut[0x46] = UniKey::KB_9;
    lut[0x48] = UniKey::KB_F21;
    lut[0x49] = UniKey::KB_DOT;
    lut[0x4A] = UniKey::KB_SLASH;
    lut[0x4B] = UniKey::KB_L;
    lut[0x4C] = UniKey::KB_SCOLON;
    lut[0x4D] = UniKey::KB_P;
    lut[0x4E] = UniKey::KB_MINUS;
    lut[0x50] = UniKey::KB_F22;
    lut[0x51] = UniKey::KB_INT1;
    lut[0x52] = UniKey::KB_QUOTE;
    lut[0x54] = UniKey::KB_LBRACKET;
    lut[0x55] = UniKey::KB_EQUAL;
    lut[0x57] = UniKey::KB_F23;
    lut[0x58] = UniKey::KB_CAPSLOCK;
    lut[0x59] = UniKey::KB_RSHIFT;
    lut[0x5A] = UniKey::KB_RETURN;
    lut[0x5B] = UniKey::KB_RBRACKET;
    lut[0x5F] = UniKey::KB_F24;
    lut[0x61] = UniKey::KB_NONUS_BSLASH;
    lut[0x62] = UniKey::KB_LANG4;
    lut[0x63] = UniKey::KB_LANG3;
    lut[0x64] = UniKey::KB_INT4;
    lut[0x66] = UniKey::KB_BSPACE;
    lut[0x67] = UniKey::KB_INT5;
    lut[0x69] = UniKey::KP_1;
    lut[0x6A] = UniKey::KB_INT3;
    lut[0x6B] = UniKey::KP_4;
    lut[0x6C] = UniKey::KP_7;
    lut[0x6D] = UniKey::KP_COMMA;
    lut[0x70] = UniKey::KP_0;
    lut[0x71] = UniKey::KP_DOT;
    lut[0x72] = UniKey::KP_2;
    lut[0x73] = UniKey::KP_5;
    lut[0x74] = UniKey::KP_6;
    lut[0x75] = UniKey::KP_8;
    lut[0x76] = UniKey::KB_ESCAPE;
    lut[0x77] = UniKey::KB_NUMLOCK;
    lut[0x78] = UniKey::KB_F11;
    lut[0x79] = UniKey::KP_PLUS;
    lut[0x7A] = UniKey::KP_3;
    lut[0x7B] = UniKey::KP_MINUS;
    lut[0x7C] = UniKey::KP_ASTERISK;
    lut[0x7D] = UniKey::KP_9;
    lut[0x7E] = UniKey::KB_SCKLOCK;

    /* Remapped region for escaped codes */
    lut[0x83] = UniKey::KB_F7;
    lut[0x84] = UniKey::KB_PSCREEN;
    lut[0x90] = UniKey::WWW_SEARCH;
    lut[0x91] = UniKey::KB_RALT;
    lut[0x92] = UniKey::KB_COMBO;
    lut[0x94] = UniKey::KB_RCTRL;
    lut[0x95] = UniKey::MEDIA_PREV_TRACK;
    lut[0x98] = UniKey::WWW_BOOKMARKS;
    lut[0x9F] = UniKey::KB_LGUI;
    lut[0xA0] = UniKey::WWW_REFRESH;
    lut[0xA1] = UniKey::MEDIA_VOL_DOWN;
    lut[0xA3] = UniKey::MEDIA_MUTE;
    lut[0xA7] = UniKey::KB_RGUI;
    lut[0xA8] = UniKey::WWW_STOP;
    lut[0xAB] = UniKey::CALCULATOR;
    lut[0xAF] = UniKey::KB_APP;
    lut[0xB0] = UniKey::WWW_FORWARD;
    lut[0xB2] = UniKey::MEDIA_VOL_UP;
    lut[0xB4] = UniKey::MEDIA_PLAY_PAUSE;
    lut[0xB7] = UniKey::KB_PWR;
    lut[0xB8] = UniKey::WWW_BACK;
    lut[0xBA] = UniKey::WWW_HOME;
    lut[0xBB] = UniKey::MEDIA_STOP;
    lut[0xBF] = UniKey::KB_SLEEP;
    lut[0xC0] = UniKey::FILES;
    lut[0xC8] = UniKey::MAIL;
    lut[0xCA] = UniKey::KP_SLASH;
    lut[0xCD] = UniKey::MEDIA_NEXT_TRACK;
    lut[0xD0] = UniKey::MEDIA_SELECT;
    lut[0xD9] = UniKey::KB_COMBO;
    lut[0xDA] = UniKey::KP_ENTER;
    lut[0xDE] = UniKey::KB_WAKE;
    lut[0xE9] = UniKey::KB_END;
    lut[0xEB] = UniKey::KB_LEFT;
    lut[0xEC] = UniKey::KB_HOME;
    lut[0xF0] = UniKey::KB_INSERT;
    lut[0xF1] = UniKey::KB_DELETE; /* UniKey::KB_LANG2 not supported */
    lut[0xF2] = UniKey::KB_DOWN;   /* UniKey::KB_LANG1 not supported */
    lut[0xF4] = UniKey::KB_RIGHT;
    lut[0xF5] = UniKey::KB_UP;
    lut[0xFA] = UniKey::KB_PGDOWN;
    lut[0xFC] = UniKey::KB_PSCREEN;
    lut[0xFD] = UniKey::KB_PGUP;
    lut[0xFE] = UniKey::KB_PAUSE;

    return lut;
  }
  UniKey _map(uint8_t code, bool escaped = false);

  State _state;
  Layout _layout;
};

#endif  // SCANCODEPARSER_H
