/**
 * @file     Keyboard.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.07.2025
 */

#ifndef KEYBOARD_TPP
#define KEYBOARD_TPP

#include "debug.h"
#include "gpio.h"

using namespace std::chrono_literals;

template <typename HwAlarm, size_t BUF_SIZE>
Keyboard<HwAlarm, BUF_SIZE>::Keyboard(SPI_TypeDef *spi, HwAlarm &hw_alarm,
                                      ScanCodeParser::Layout layout)
    : _spi(spi), _hw_alarm(hw_alarm), _scparser(layout) {}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::setPins(const PinCfg &miso,
                                          const PinCfg &mosi,
                                          const PinCfg &sclk,
                                          const PinCfg &nss) {
  _miso = miso;
  _mosi = mosi;
  _sclk = sclk;
  _nss = nss;
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::setFormat(uint32_t cpol, uint32_t cpha) {
  _cpol = cpol;
  _cpha = cpha;
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::setBaudRate(uint32_t baud_rate) {
  _baud_rate = baud_rate;
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::_init() const {
  /* Check peripheral availability */
  spi::enableClock(_spi);
  if (LL_SPI_IsEnabled(_spi)) {
    PRINTE("SPI already enabled: uncaught conflict");
    exit(-4);
  }

  /* Initialize GPIO peripheral */
  for (const auto &cfg : {_miso, _mosi, _sclk}) {
    gpio::enableClock(cfg.gpio);
    LL_GPIO_InitTypeDef gpio_init{
        .Pin = cfg.pin,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_LOW,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = cfg.af};
    LL_GPIO_Init(cfg.gpio, &gpio_init);
  }
  LL_GPIO_SetPinPull(_miso.gpio, _miso.pin, LL_GPIO_PULL_DOWN);

  /* Initialize SPI peripheral */
  spi::enableClock(_spi);
  LL_SPI_InitTypeDef spi_init{.TransferDirection = LL_SPI_FULL_DUPLEX,
                              .Mode = LL_SPI_MODE_MASTER,
                              .DataWidth = LL_SPI_DATAWIDTH_16BIT,
                              .ClockPolarity = _cpol,
                              .ClockPhase = _cpha,
                              .NSS = LL_SPI_NSS_SOFT,
                              .BaudRate = _baud_rate,
                              .BitOrder = LL_SPI_MSB_FIRST,
                              .CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE};
  LL_SPI_Init(_spi, &spi_init);

  /* Enable SPI in master mode */
  LL_SPI_SetStandard(_spi, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_Enable(_spi);
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::_deinit() const {
  /* Wait for SPI activity to terminate */
  while (!LL_SPI_IsActiveFlag_RXNE(_spi));
  while (!LL_SPI_IsActiveFlag_TXE(_spi));
  while (!LL_SPI_IsActiveFlag_BSY(_spi));

  /* Disable SPI */
  LL_SPI_DeInit(_spi);
  for (const auto &cfg : {_miso, _mosi, _sclk}) {
    LL_GPIO_InitTypeDef gpio_init{.Pin = cfg.pin, .Mode = LL_GPIO_MODE_ANALOG};
    LL_GPIO_Init(cfg.gpio, &gpio_init);
  }
}

template <typename HwAlarm, size_t BUF_SIZE>
uint16_t Keyboard<HwAlarm, BUF_SIZE>::_txrx(uint16_t mosi_data) const {
  /* Slave device updates data on NSS edges */
  constexpr size_t nselect = 5;

  /* Select slave */
  for (size_t i = 0; i < nselect; ++i) _nss.gpio->BSRR = _nss.pin << 16;

  while (!LL_SPI_IsActiveFlag_TXE(_spi));
  LL_SPI_TransmitData16(_spi, mosi_data);

  while (!LL_SPI_IsActiveFlag_RXNE(_spi));
  const auto rply = LL_SPI_ReceiveData16(_spi);

  /* Deselect slave */
  for (size_t i = 0; i < nselect; ++i) _nss.gpio->BSRR = _nss.pin;

  PRINTD(
      "SPI Keyboard Slave: 0x%04X\n"
      "SPI Master        : 0x%04X",
      rply, mosi_data);

  return rply;
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::_reset(bool enable) {
  /* Restart slave device */
  _txrx(MOSI_WEN | (enable & MOSI_CEN) | MOSI_BCLR);

  /* Restart parsing chain */
  _scparser.reset();
  _kmapper.reset();
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::reset(bool enable) {
  _init();
  _reset(enable);
  _deinit();
}

template <typename HwAlarm, size_t BUF_SIZE>
auto Keyboard<HwAlarm, BUF_SIZE>::_peek() -> CharT {
  CharT c{};

  /* Already initialized ... */
  const auto rply = _txrx();

  if (!(rply & MISO_EN)) {
    PRINTD("PS2 controller disabled. Enabling ...");
    _txrx(MOSI_WEN | MOSI_CEN | MOSI_BCLR);
  } else if (rply & (MISO_FE | MISO_PE | MISO_CTO | MISO_RTO)) {
    PRINTE("Communication error. Resetting ...");
    _reset();
  } else {
    if (rply & MISO_OE) PRINTD("Overrun error detected");

    if (rply & MISO_RXV) {
      const auto pk = _scparser.parse(static_cast<uint8_t>(rply >> 8));
      c = _kmapper.put(pk);
    }
  }
  return c;
}

template <typename HwAlarm, size_t BUF_SIZE>
bool Keyboard<HwAlarm, BUF_SIZE>::available() {
  _init();
  const auto c = _peek();
  _deinit();

  return c != CharT{};
}

template <typename HwAlarm, size_t BUF_SIZE>
auto Keyboard<HwAlarm, BUF_SIZE>::getLine() -> const
    typename KeyMapper<BUF_SIZE>::String & {
  CharT c;

  _init();
  while ((c = _peek()) != '\n') {
    typename HwAlarm::Cnt cticks;
    _hw_alarm.getTick(500ms, cticks);
    while (cticks > _hw_alarm.now());
  }
  _deinit();

  return _kmapper.get();
}

template <typename HwAlarm, size_t BUF_SIZE>
void Keyboard<HwAlarm, BUF_SIZE>::clear() {
  _kmapper.reset();
}


#endif  // KEYBOARD_TPP
