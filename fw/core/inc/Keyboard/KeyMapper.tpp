/**
 * @file     KeyMapper.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     11.07.2025
 * @brief    Maps a subset of parsed keys into characters. Parsable keys
 *           are: keyboard and keypad digits, plus, minus, dot, and
 *           backspace. The terminator parsable keys are enter and return.
 */

#ifndef KEYMAPPER_TPP
#define KEYMAPPER_TPP

template <size_t BUF_SIZE>
void KeyMapper<BUF_SIZE>::reset() {
  _str.clear();
}

template <size_t BUF_SIZE>
auto KeyMapper<BUF_SIZE>::put(ScanCodeParser::ParsedKey pk) -> CharT {
  CharT c{};
  if (pk.action == ScanCodeParser::RESET)
    reset();
  else if (pk.action == ScanCodeParser::MAKE) {
    if ((c = CharLut()[static_cast<uint8_t>(pk.key)]) != CharT{}) {
      _str.push(c);
    }
  }
  return c;
}

template <size_t BUF_SIZE>
auto KeyMapper<BUF_SIZE>::get() -> const String& {
  _str.linearize();
  return _str;
}

#endif //KEYMAPPER_TPP
