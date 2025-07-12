/**
 * @file     ScanCodeParser.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     11.07.2025
 * @brief    Parser for Scan Code Set 2
 * @see      Keyboards (101- and 102-Key)
 *           http://www.mcamafia.de/pdf/ibm_hitrc11.pdf
 *           Microsoft USB HID to PS/2 Translation Table - Scan Code Set 1 and 2
 *           http://download.microsoft.com/download/1/6/1/161ba512-40e2-4cc9-843a-923143f3456c/translate.pdf
 */

#include "ScanCodeParser.h"

#include "debug.h"

ScanCodeParser::ScanCodeParser(Layout layout)
    : _state(MAKE_START), _layout(layout) {}

void ScanCodeParser::setLayout(Layout layout) {
  _layout = layout;
  reset();
}

void ScanCodeParser::reset() { _state = MAKE_START; }

UniKey ScanCodeParser::_map(uint8_t code, bool escaped) {
  /* Escaped codes are mapped into the unused region > 0x80 */
  if (escaped) code |= 0x80;

  /* Layout adjustment */
  if (code == 0x5D)
    return _layout == NONUS_102K ? UniKey::KB_NONUS_HASH : UniKey::KB_BSLASH;

  return KeysLut()[code];
}

ScanCodeParser::ParsedKey ScanCodeParser::parse(uint8_t code) {
  if (code == 0x00) {
    PRINTE("Overrun error. Forcing reset...");
    _state = MAKE_START;
    return {.action = RESET};
  }

  if (code == 0xAA || code == 0xFC) {
    PRINTD("BAT: %s", code == 0xAA ? "passed" : "failed");
    _state = MAKE_START;
    return {.action = RESET};
  }

  ParsedKey pk{.action = NONE};

  switch (_state) {
    case MAKE_START: {
      switch (code) {
        case 0xE0:
          _state = MAKE_E0;
          break;
        case 0xE1:
          _state = MAKE_E1;
          break;
        case 0xF0:
          _state = BREAK_F0;
          break;
        default:
          _state = MAKE_START;
          if (const auto key = _map(code);
              key == UniKey::KB_NONE || key == UniKey::KB_COMBO) {
            PRINTE("Unexpected code @ MAKE_START: 0x%02X", code);
            pk.action = RESET;
          } else {
            pk.action = MAKE;
            pk.key = key;
          }
      }
    } break;

    case BREAK_F0: {
      _state = MAKE_START;
      if (const auto key = _map(code);
          key == UniKey::KB_NONE || key == UniKey::KB_COMBO) {
        PRINTE("Unexpected code @ BREAK_F0: 0x%02X", code);
        pk.action = RESET;
      } else {
        pk.action = BREAK;
        pk.key = key;
      }
    } break;

    case MAKE_E0: {
      if (code == 0xF0)
        _state = BREAK_E0_F0;
      else {
        _state = MAKE_START;
        if (const auto key = _map(code, true); key == UniKey::KB_NONE) {
          PRINTE("Unexpected code @ MAKE_E0: 0x%02X", code);
          pk.action = RESET;
        } else if (key != UniKey::KB_COMBO) {
          pk.action = MAKE;
          pk.key = key;
        }
      }
    } break;

    case BREAK_E0_F0: {
      _state = MAKE_START;
      if (const auto key = _map(code, true); key == UniKey::KB_NONE) {
        PRINTE("Unexpected code @ BREAK_E0_F0: 0x%02X", code);
        pk.action = RESET;
      } else if (key != UniKey::KB_COMBO) {
        pk.action = BREAK;
        pk.key = key;
      }
    } break;

    case MAKE_E1: {
      if (code == 0x14)
        _state = MAKE_E1_14;
      else {
        PRINTE("Unexpected code @ MAKE_E1: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case MAKE_E1_14: {
      if (code == 0x77) {
        _state = MAKE_E1_14_77;
        pk.action = MAKE;
        pk.key = UniKey::KB_PAUSE;
      } else {
        PRINTE("Unexpected code @ MAKE_E1_14: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case MAKE_E1_14_77: {
      if (code == 0xE1)
        _state = BREAK_E1;
      else {
        PRINTE("Unexpected code @ MAKE_E1_14_77: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case BREAK_E1: {
      if (code == 0xF0)
        _state = BREAK_E1_F0;
      else {
        PRINTE("Unexpected code @ BREAK_E1: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case BREAK_E1_F0: {
      if (code == 0x14)
        _state = BREAK_E1_F0_14;
      else {
        PRINTE("Unexpected code @ BREAK_E1_F0: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case BREAK_E1_F0_14: {
      if (code == 0xF0)
        _state = BREAK_E1_F0_14_F0;
      else {
        PRINTE("Unexpected code @ BREAK_E1_F0_14: 0x%02X", code);
        _state = MAKE_START;
        pk.action = RESET;
      }
    } break;

    case BREAK_E1_F0_14_F0: {
      _state = MAKE_START;
      if (code == 0x77) {
        pk.action = BREAK;
        pk.key = UniKey::KB_PAUSE;
      } else {
        PRINTE("Unexpected code @ BREAK_E1_F0_14_F0: 0x%02X", code);
        pk.action = RESET;
      }
    } break;
  }

  return pk;
}
