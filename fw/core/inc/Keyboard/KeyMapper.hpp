/**
 * @file     KeyMapper.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     11.07.2025
 * @brief    Maps a subset of parsed keys into characters. Parsable keys
 *           are: keyboard and keypad digits, plus, minus, dot, and
 *           backspace. The terminator parsable keys are enter and return.
 */

#ifndef KEYMAPPER_HPP
#define KEYMAPPER_HPP

#include "FifoArray.hpp"
#include "ScanCodeParser.h"

template <size_t BUF_SIZE>
class KeyMapper {
 public:
  using CharT = char;
  using String = FifoArray<CharT, BUF_SIZE>;

  void reset();
  CharT put(ScanCodeParser::ParsedKey pk);
  const String& get();

 private:
  static constexpr auto CharLut() {
    std::array<CharT, 256> lut{};

    lut[static_cast<uint8_t>(UniKey::KB_1)] =
        lut[static_cast<uint8_t>(UniKey::KP_1)] = '1';
    lut[static_cast<uint8_t>(UniKey::KB_2)] =
        lut[static_cast<uint8_t>(UniKey::KP_2)] = '2';
    lut[static_cast<uint8_t>(UniKey::KB_3)] =
        lut[static_cast<uint8_t>(UniKey::KP_3)] = '3';
    lut[static_cast<uint8_t>(UniKey::KB_4)] =
        lut[static_cast<uint8_t>(UniKey::KP_4)] = '4';
    lut[static_cast<uint8_t>(UniKey::KB_5)] =
        lut[static_cast<uint8_t>(UniKey::KP_5)] = '5';
    lut[static_cast<uint8_t>(UniKey::KB_6)] =
        lut[static_cast<uint8_t>(UniKey::KP_6)] = '6';
    lut[static_cast<uint8_t>(UniKey::KB_7)] =
        lut[static_cast<uint8_t>(UniKey::KP_7)] = '7';
    lut[static_cast<uint8_t>(UniKey::KB_8)] =
        lut[static_cast<uint8_t>(UniKey::KP_8)] = '8';
    lut[static_cast<uint8_t>(UniKey::KB_9)] =
        lut[static_cast<uint8_t>(UniKey::KP_9)] = '9';
    lut[static_cast<uint8_t>(UniKey::KB_0)] =
        lut[static_cast<uint8_t>(UniKey::KP_0)] = '0';

    lut[static_cast<uint8_t>(UniKey::KB_RBRACKET)] =
        lut[static_cast<uint8_t>(UniKey::KP_PLUS)] = '+';

    lut[static_cast<uint8_t>(UniKey::KB_SLASH)] =
        lut[static_cast<uint8_t>(UniKey::KP_MINUS)] = '-';

    lut[static_cast<uint8_t>(UniKey::KB_DOT)] =
        lut[static_cast<uint8_t>(UniKey::KP_DOT)] = '.';

    lut[static_cast<uint8_t>(UniKey::KB_BSPACE)] = '\b';

    lut[static_cast<uint8_t>(UniKey::KB_RETURN)] =
        lut[static_cast<uint8_t>(UniKey::KP_ENTER)] = '\n';

    return lut;
  }

  String _str;
};

#include "KeyMapper.tpp"

#endif  // KEYMAPPER_HPP
