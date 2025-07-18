/**
 * @file     SpiMaster.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     15.07.2025
 */

#ifndef SPIMASTER_TPP
#define SPIMASTER_TPP

#include <algorithm>
#include <limits>

#include "debug.h"

template <typename HwAlarm, size_t NMAX_SLAVES>
SpiMaster<HwAlarm, NMAX_SLAVES>::SpiMaster(SPI_TypeDef *spi, HwAlarm &hw_alarm)
    : _spi(spi),
      _hw_alarm(hw_alarm),
      _sclk{},
      _slaves{},
      _active_cfg(_slaves.size()) {}

template <typename HwAlarm, size_t NMAX_SLAVES>
void SpiMaster<HwAlarm, NMAX_SLAVES>::init(const AltPinCfg &miso,
                                           const AltPinCfg &mosi,
                                           const AltPinCfg &sclk) {
  /* Initialize GPIO peripheral.
   * While the SPI peripheral is disabled, the AF GPIO pins are not driven;
   * also, when all slaves are deselected, the MISO pin is in 'z.
   * The weak pull resistors are used to establish a valid logic state:
   * for now there is no info about desired CPOL.
   */

  for (const auto &cfg : {miso, mosi, sclk}) {
    gpio::enableClock(cfg.pin_cfg.gpio);
    LL_GPIO_InitTypeDef gpio_init{
        .Pin = cfg.pin_cfg.pin,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_DOWN,
        .Alternate = cfg.af,
    };
    LL_GPIO_Init(cfg.pin_cfg.gpio, &gpio_init);
  }
  _sclk = sclk.pin_cfg;

  /* Initialize SPI peripheral */
  spi::enableClock(_spi);
  LL_SPI_SetTransferDirection(_spi, LL_SPI_FULL_DUPLEX);
  LL_SPI_SetMode(_spi, LL_SPI_MODE_MASTER);
  LL_SPI_SetNSSMode(_spi, LL_SPI_NSS_SOFT);
  LL_SPI_SetTransferBitOrder(_spi, LL_SPI_MSB_FIRST);
  LL_SPI_SetStandard(_spi, LL_SPI_PROTOCOL_MOTOROLA);
  LL_SPI_DisableCRC(_spi);
}

template <typename HwAlarm, size_t NMAX_SLAVES>
auto SpiMaster<HwAlarm, NMAX_SLAVES>::addSlave(const PinCfg &ssn, size_t nbits,
                                               bool cpol, bool cpha,
                                               ClockFreq max_fclk_hz)
    -> std::optional<SlaveId> {
  /* Check compatibility with frame formats */
  if (nbits > std::numeric_limits<FrameT>::digits) return {};

  /* Compute baud rate settings, if a feasible solution exists */
  auto fclk_hz = spi::getAPBClockFreq(_spi) >> 1;
  uint8_t br_psc = 0;

  constexpr auto br_psc_end = 8;
  while (fclk_hz > max_fclk_hz && br_psc < br_psc_end) {
    br_psc++;
    fclk_hz >>= 1;
  }
  if (br_psc == br_psc_end) return {};

  /* Look for a free slave id */
  const auto it =
      std::find_if(_slaves.begin(), _slaves.end(),
                   [](const Slave &scfg) { return !scfg.ssn.gpio; });
  if (it == _slaves.end()) return {};

  /* Store the slave configuration */
  it->ssn = ssn;
  it->nbits = nbits;
  it->cpol = cpol;
  it->cpha = cpha;
  it->br_psc = br_psc;

  /* Initialize GPIO peripheral */
  gpio::enableClock(ssn.gpio);

  LL_GPIO_InitTypeDef gpio_init{
      .Pin = ssn.pin,
      .Mode = LL_GPIO_MODE_OUTPUT,
      .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
      .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
      .Pull = LL_GPIO_PULL_NO,
  };
  LL_GPIO_SetOutputPin(ssn.gpio, ssn.pin);  // make sure it's deselected
  LL_GPIO_Init(ssn.gpio, &gpio_init);

  /* The id is the index into _slaves */
  const auto id = std::distance(_slaves.begin(), it);

  PRINTD("Slave configured [fclk_hz = %u, id = %u]", fclk_hz, id);
  return {id};
}

template <typename HwAlarm, size_t NMAX_SLAVES>
bool SpiMaster<HwAlarm, NMAX_SLAVES>::_isValid(SlaveId sid) const {
  return sid < _slaves.size() && _slaves[sid].ssn.gpio != nullptr;
}

template <typename HwAlarm, size_t NMAX_SLAVES>
void SpiMaster<HwAlarm, NMAX_SLAVES>::removeSlave(SlaveId sid) {
  if (!_isValid(sid)) return;

  /* SPI transfers are blocking by design
   * (No need to check activity or select state) */
  LL_GPIO_InitTypeDef gpio_init{
      .Pin = _slaves[sid].ssn.pin,
      .Mode = LL_GPIO_MODE_ANALOG,
  };
  LL_GPIO_Init(_slaves[sid].ssn.gpio, &gpio_init);

  /* The removed slave may be replaced: reset last active */
  _slaves[sid].ssn.gpio = nullptr;
  if (sid == _active_cfg) _active_cfg = _slaves.size();
}

template <typename HwAlarm, size_t NMAX_SLAVES>
void SpiMaster<HwAlarm, NMAX_SLAVES>::_applyCfg(SlaveId sid) {
  const auto &slave = _slaves[sid];

  /* Match weak pull resistors to CPOL */
  LL_GPIO_SetPinPull(_sclk.gpio, _sclk.pin,
                     slave.cpol ? LL_GPIO_PULL_UP : LL_GPIO_PULL_DOWN);
  _hw_alarm.delay(T_WEAK_PUPD);

  /* Match SPI peripheral configuration for this slave */
  LL_SPI_SetDataWidth(
      _spi, slave.nbits <= 8 ? LL_SPI_DATAWIDTH_8BIT : LL_SPI_DATAWIDTH_16BIT);
  LL_SPI_SetClockPolarity(
      _spi, slave.cpol ? LL_SPI_POLARITY_HIGH : LL_SPI_POLARITY_LOW);
  LL_SPI_SetClockPhase(_spi,
                       slave.cpha ? LL_SPI_PHASE_2EDGE : LL_SPI_PHASE_1EDGE);
  LL_SPI_SetBaudRatePrescaler(_spi, slave.br_psc << SPI_CR1_BR_Pos);

  _active_cfg = sid;
}

template <typename HwAlarm, size_t NMAX_SLAVES>
std::optional<typename SpiMaster<HwAlarm, NMAX_SLAVES>::FrameT>
SpiMaster<HwAlarm, NMAX_SLAVES>::txrx(SlaveId sid, FrameT txd,
                                      HwAlarmType::NanoSeconds t_pre,
                                      HwAlarmType::NanoSeconds t_post) {
  if (!_isValid(sid)) return {};

  const auto &slave = _slaves[sid];

  /* Configure SPI only if the slave has changed */
  if (_active_cfg != sid) _applyCfg(sid);

  /* !! Align data (slave.nbit wide) to the MSB of the frame (8/16 bit wide) */
  const auto frame_nbits = slave.nbits <= 8 ? 8 : 16;
  txd <<= frame_nbits - slave.nbits;

  /* 1) Select */
  LL_GPIO_ResetOutputPin(slave.ssn.gpio, slave.ssn.pin);
  _hw_alarm.delay(t_pre);  // Blocking operation

  /* 2) TX <---> RX */
  LL_SPI_Enable(_spi);
  if (frame_nbits == 8)
    LL_SPI_TransmitData8(_spi, static_cast<uint8_t>(txd));
  else
    LL_SPI_TransmitData16(_spi, txd);

  while (!LL_SPI_IsActiveFlag_RXNE(_spi));
  auto rply{frame_nbits == 8 ? static_cast<FrameT>(LL_SPI_ReceiveData8(_spi))
                             : LL_SPI_ReceiveData16(_spi)};

  // TXE already 1
  while (LL_SPI_IsActiveFlag_BSY(_spi));
  LL_SPI_Disable(_spi);

  /* 3) Deselect */
  _hw_alarm.delay(t_post);  // Blocking operation
  LL_GPIO_SetOutputPin(slave.ssn.gpio, slave.ssn.pin);

  /* !! Align data (slave.nbit wide) to the LSB of the frame (8/16 bit wide) */
  rply >>= frame_nbits - slave.nbits;
  PRINTD("Master [0x%0*X] <---> [0x%0*X] Slave #%u", frame_nbits / 4, txd,
         frame_nbits / 4, rply, sid);

  return {rply};
}

#endif  // SPIMASTER_TPP