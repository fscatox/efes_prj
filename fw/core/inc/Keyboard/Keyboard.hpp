/**
 * @file     Keyboard.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     17.07.2025
 */

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "IFile.h"
#include "ScanCodeParser.h"

template <typename SpiMaster, typename HwAlarm>
class Keyboard : public IFile {
 public:
  using Layout = ScanCodeParser::Layout;

  Keyboard(SpiMaster& spi, HwAlarm& hw_alarm,
           Layout layout = Layout::NONUS_102K);

  void setSsn(GPIO_TypeDef* gpio, uint32_t pin);
  void setMaxClockFreq(typename SpiMaster::ClockFreq fclk_hz);
  void setOptions(bool cpol, bool cpha);

  int open(OFile& ofile) override;
  ssize_t read(OFile& ofile, char* buf, size_t count, off_t& pos) override;
  __poll_t poll(OFile& ofile) override;

 private:
  using FrameT = uint16_t;

  struct State {
    bool lshift, rshift;
    bool alt_gr;
    bool caps_lock;
    bool num_lock;
  };

  constexpr static auto T_SYNC = typename HwAlarm::NanoSeconds{200};
  constexpr static auto T_RST = typename HwAlarm::NanoSeconds{200};
  constexpr static auto T_POLL = typename HwAlarm::MicroSeconds{2};

  constexpr static FrameT MOSI_WEN = 0x01 << 0;  /*!< Write Enable */
  constexpr static FrameT MOSI_CEN = 0x01 << 1;  /*!< Controller Enable */
  constexpr static FrameT MOSI_BCLR = 0x01 << 2; /*!< Buffer Clear */
  constexpr static FrameT MOSI_TXEN = 0x01 << 7; /*!< Transmitter Mode Enable */
  constexpr static FrameT MOSI_RST = (MOSI_WEN | MOSI_CEN | MOSI_BCLR);

  constexpr static FrameT MISO_EN = 0x01 << 0;  /*!< Controller Enabled */
  constexpr static FrameT MISO_OE = 0x01 << 1;  /*!< Overrun Error */
  constexpr static FrameT MISO_FE = 0x01 << 2;  /*!< Frame Error */
  constexpr static FrameT MISO_PE = 0x01 << 3;  /*!< Parity Error */
  constexpr static FrameT MISO_CTO = 0x01 << 4; /*!< Clock Timeout */
  constexpr static FrameT MISO_RTO = 0x01 << 5; /*!< Request Timeout */
  constexpr static FrameT MISO_TXC = 0x01 << 6; /*!< Transmission Complete */
  constexpr static FrameT MISO_RXV = 0x01 << 7; /*!< Receive Data Valid */

  /*
   * Lookup table for ASCII mappings with Shift and AltGr modifiers
   *   [0] Base key
   *   [1] Shift + key
   *   [2] AltGr + key
   *   [3] AltGr + shift + key
   */
  static constexpr auto AsciiLut() {
    std::array<std::array<char, 4>, 256> lut{};

    lut[static_cast<uint8_t>(UniKey::KB_GRAVE)] = {'\\', '|'};
    lut[static_cast<uint8_t>(UniKey::KB_1)] = {'1', '!'};
    lut[static_cast<uint8_t>(UniKey::KB_2)] = {'2', '"'};
    lut[static_cast<uint8_t>(UniKey::KB_3)] = {'3'};
    lut[static_cast<uint8_t>(UniKey::KB_4)] = {'4', '$'};
    lut[static_cast<uint8_t>(UniKey::KB_5)] = {'5', '%'};
    lut[static_cast<uint8_t>(UniKey::KB_6)] = {'6', '&'};
    lut[static_cast<uint8_t>(UniKey::KB_7)] = {'7', '/', '{'};
    lut[static_cast<uint8_t>(UniKey::KB_8)] = {'8', '(', '['};
    lut[static_cast<uint8_t>(UniKey::KB_9)] = {'9', ')', ']'};
    lut[static_cast<uint8_t>(UniKey::KB_0)] = {'0', '=', '}'};
    lut[static_cast<uint8_t>(UniKey::KB_MINUS)] = {'\'', '?', '`'};
    lut[static_cast<uint8_t>(UniKey::KB_EQUAL)] = {'\0', '^', '~'};
    lut[static_cast<uint8_t>(UniKey::KB_TAB)] = {'\t'};
    lut[static_cast<uint8_t>(UniKey::KB_Q)] = {'q', 'Q', '@'};
    lut[static_cast<uint8_t>(UniKey::KB_W)] = {'w', 'W'};
    lut[static_cast<uint8_t>(UniKey::KB_E)] = {'e', 'E'};
    lut[static_cast<uint8_t>(UniKey::KB_R)] = {'r', 'R'};
    lut[static_cast<uint8_t>(UniKey::KB_T)] = {'t', 'T'};
    lut[static_cast<uint8_t>(UniKey::KB_Y)] = {'y', 'Y'};
    lut[static_cast<uint8_t>(UniKey::KB_U)] = {'u', 'U'};
    lut[static_cast<uint8_t>(UniKey::KB_I)] = {'i', 'I'};
    lut[static_cast<uint8_t>(UniKey::KB_O)] = {'o', 'O'};
    lut[static_cast<uint8_t>(UniKey::KB_P)] = {'p', 'P'};
    lut[static_cast<uint8_t>(UniKey::KB_LBRACKET)] = {'\0', '\0', '[', '{'};
    lut[static_cast<uint8_t>(UniKey::KB_RBRACKET)] = {'+', '*', ']', '}'};
    lut[static_cast<uint8_t>(UniKey::KB_A)] = {'a', 'A'};
    lut[static_cast<uint8_t>(UniKey::KB_S)] = {'s', 'S'};
    lut[static_cast<uint8_t>(UniKey::KB_D)] = {'d', 'D'};
    lut[static_cast<uint8_t>(UniKey::KB_F)] = {'f', 'F'};
    lut[static_cast<uint8_t>(UniKey::KB_G)] = {'g', 'G'};
    lut[static_cast<uint8_t>(UniKey::KB_H)] = {'h', 'H'};
    lut[static_cast<uint8_t>(UniKey::KB_J)] = {'j', 'J'};
    lut[static_cast<uint8_t>(UniKey::KB_K)] = {'k', 'K', '\0', '&'};
    lut[static_cast<uint8_t>(UniKey::KB_L)] = {'l', 'L'};
    lut[static_cast<uint8_t>(UniKey::KB_SCOLON)] = {'\0', '\0', '@'};
    lut[static_cast<uint8_t>(UniKey::KB_QUOTE)] = {'\0', '\0', '#'};
    lut[static_cast<uint8_t>(UniKey::KB_RETURN)] = {'\n', '\n', '\n', '\n'};
    lut[static_cast<uint8_t>(UniKey::KB_NONUS_BSLASH)] = {'<', '>'};
    lut[static_cast<uint8_t>(UniKey::KB_Z)] = {'z', 'Z', '\0', '<'};
    lut[static_cast<uint8_t>(UniKey::KB_X)] = {'x', 'X', '\0', '>'};
    lut[static_cast<uint8_t>(UniKey::KB_C)] = {'c', 'C'};
    lut[static_cast<uint8_t>(UniKey::KB_V)] = {'v', 'V'};
    lut[static_cast<uint8_t>(UniKey::KB_B)] = {'b', 'B'};
    lut[static_cast<uint8_t>(UniKey::KB_N)] = {'n', 'N'};
    lut[static_cast<uint8_t>(UniKey::KB_M)] = {'m', 'M'};
    lut[static_cast<uint8_t>(UniKey::KB_COMMA)] = {',', ';'};
    lut[static_cast<uint8_t>(UniKey::KB_DOT)] = {'.', ':'};
    lut[static_cast<uint8_t>(UniKey::KB_SLASH)] = {'-', '_'};
    lut[static_cast<uint8_t>(UniKey::KB_SPACE)] = {' ', ' ', ' ', ' '};
    lut[static_cast<uint8_t>(UniKey::KP_7)] = {'7', '\0', '7'};
    lut[static_cast<uint8_t>(UniKey::KP_4)] = {'4', '\0', '4'};
    lut[static_cast<uint8_t>(UniKey::KP_1)] = {'1', '\0', '1'};
    lut[static_cast<uint8_t>(UniKey::KP_SLASH)] = {'/', '/', '/', '/'};
    lut[static_cast<uint8_t>(UniKey::KP_8)] = {'8', '\0', '8'};
    lut[static_cast<uint8_t>(UniKey::KP_5)] = {'5', '\0', '5'};
    lut[static_cast<uint8_t>(UniKey::KP_2)] = {'2', '\0', '2'};
    lut[static_cast<uint8_t>(UniKey::KP_0)] = {'0', '\0', '0'};
    lut[static_cast<uint8_t>(UniKey::KP_ASTERISK)] = {'*', '*', '*', '*'};
    lut[static_cast<uint8_t>(UniKey::KP_9)] = {'9', '\0', '9'};
    lut[static_cast<uint8_t>(UniKey::KP_6)] = {'6', '\0', '6'};
    lut[static_cast<uint8_t>(UniKey::KP_3)] = {'3', '\0', '3'};
    lut[static_cast<uint8_t>(UniKey::KP_DOT)] = {'.', '\0', '.'};
    lut[static_cast<uint8_t>(UniKey::KP_MINUS)] = {'-', '-', '-', '-'};
    lut[static_cast<uint8_t>(UniKey::KP_PLUS)] = {'+', '+', '+', '+'};
    lut[static_cast<uint8_t>(UniKey::KP_COMMA)] = {',', ',', ',', ','};
    lut[static_cast<uint8_t>(UniKey::KP_ENTER)] = {'\n', '\n', '\n', '\n'};
    lut[static_cast<uint8_t>(UniKey::KP_EQUAL)] = {'=', '=', '=', '='};

    return lut;
  }

  char _map(UniKey k) const;

  int _tryRead(uint8_t& scan_code) const;
  char _step(uint8_t scan_code);

  int _getc(char& c);

  SpiMaster& _spi;
  HwAlarm& _hw_alarm;

  typename SpiMaster::SlaveId _id;
  typename SpiMaster::PinCfg _ssn;
  bool _cpol, _cpha;
  typename SpiMaster::ClockFreq _max_fclk_hz;

  ScanCodeParser _parser;
  State _state;
  char _peek;
};

#include "Keyboard.tpp"


#endif  // KEYBOARD_HPP
