/**
 * @file     Keyboard.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.07.2025
 */

#ifndef KEYBOARD_HPP
#define KEYBOARD_HPP

#include "KeyMapper.hpp"
#include "ScanCodeParser.h"
#include "spi.h"

template <typename HwAlarm, size_t BUF_SIZE = 32>
class Keyboard {
 public:
  using CharT = typename KeyMapper<BUF_SIZE>::CharT;

  struct PinCfg {
    GPIO_TypeDef *gpio;
    uint32_t pin;
    uint32_t af;
  };

  Keyboard(SPI_TypeDef *spi, HwAlarm &hw_alarm,
           ScanCodeParser::Layout layout = ScanCodeParser::NONUS_102K);

  void setPins(const PinCfg &miso, const PinCfg &mosi, const PinCfg &sclk,
               const PinCfg &nss);
  void setFormat(uint32_t cpol, uint32_t cpha);
  void setBaudRate(uint32_t baud_rate);

  bool available();
  const typename KeyMapper<BUF_SIZE>::String &getLine();
  void clear();

  void reset(bool enable = true);

 private:
  using MilliSeconds = typename HwAlarm::MilliSeconds;

  constexpr static uint16_t MOSI_WEN = 0x01 << 0;  /*!< Write Enable */
  constexpr static uint16_t MOSI_CEN = 0x01 << 1;  /*!< Controller Enable */
  constexpr static uint16_t MOSI_BCLR = 0x01 << 2; /*!< Buffer Clear */
  constexpr static uint16_t MOSI_TXEN = 0x01
                                        << 7; /*!< Transmitter Mode Enable */

  constexpr static uint16_t MISO_EN = 0x01 << 0;  /*!< Controller Enabled */
  constexpr static uint16_t MISO_OE = 0x01 << 1;  /*!< Overrun Error */
  constexpr static uint16_t MISO_FE = 0x01 << 2;  /*!< Frame Error */
  constexpr static uint16_t MISO_PE = 0x01 << 3;  /*!< Parity Error */
  constexpr static uint16_t MISO_CTO = 0x01 << 4; /*!< Clock Timeout */
  constexpr static uint16_t MISO_RTO = 0x01 << 5; /*!< Request Timeout */
  constexpr static uint16_t MISO_TXC = 0x01 << 6; /*!< Transmission Complete */
  constexpr static uint16_t MISO_RXV = 0x01 << 7; /*!< Receive Data Valid */

  void _init() const;
  void _deinit() const;
  uint16_t _txrx(uint16_t mosi_data = 0) const;

  void _reset(bool enable = true);
  CharT _peek();

  SPI_TypeDef *_spi;
  HwAlarm &_hw_alarm;
  PinCfg _miso, _mosi, _sclk, _nss;
  uint32_t _cpol, _cpha;
  uint32_t _baud_rate;
  ScanCodeParser _scparser;
  KeyMapper<BUF_SIZE> _kmapper;
};

#include "Keyboard.tpp"

#endif  // KEYBOARD_HPP
