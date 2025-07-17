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
    : _spi(spi), _hw_alarm(hw_alarm), _slaves{}, _active_cfg(_slaves.size()) {}

template <typename HwAlarm, size_t NMAX_SLAVES>
void SpiMaster<HwAlarm, NMAX_SLAVES>::init(const AltPinCfg &miso,
                                           const AltPinCfg &mosi,
                                           const AltPinCfg &sclk) {
  /* Initialize GPIO peripheral */
  for (const auto &cfg : {miso, mosi, sclk}) {
    gpio::enableClock(cfg.pin_cfg.gpio);
    LL_GPIO_InitTypeDef gpio_init{
        .Pin = cfg.pin_cfg.pin,
        .Mode = LL_GPIO_MODE_ALTERNATE,
        .Speed = LL_GPIO_SPEED_FREQ_MEDIUM,
        .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
        .Pull = LL_GPIO_PULL_NO,
        .Alternate = cfg.af,
    };
    LL_GPIO_Init(cfg.pin_cfg.gpio, &gpio_init);
  }

  /* When all slaves are deselected, pull MISO down */
  LL_GPIO_SetPinPull(miso.pin_cfg.gpio, miso.pin_cfg.pin, LL_GPIO_PULL_DOWN);

  /* Initialize SPI peripheral */
  spi::enableClock(_spi);
  LL_SPI_SetTransferDirection(_spi, LL_SPI_FULL_DUPLEX);
  LL_SPI_SetMode(_spi, LL_SPI_MODE_MASTER);
  LL_SPI_SetNSSMode(_spi, LL_SPI_NSS_SOFT);
  LL_SPI_SetTransferBitOrder(_spi, LL_SPI_MSB_FIRST);
  LL_SPI_DisableCRC(_spi);
  LL_SPI_SetStandard(_spi, LL_SPI_PROTOCOL_MOTOROLA);
}

template <typename HwAlarm, size_t NMAX_SLAVES>
auto SpiMaster<HwAlarm, NMAX_SLAVES>::addSlave(const PinCfg &ssn, size_t nbit,
                                               bool cpol, bool cpha,
                                               ClockFreq max_fclk_hz)
    -> std::optional<SlaveId> {
  /* Check compatibility with frame formats */
  if (nbit > std::numeric_limits<DataT>::digits) return {};

  /* Compute baud rate settings, if feasible solution exists */
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
  *it = {
      .ssn = ssn,
      .nbit = nbit,
      .cpol = cpol,
      .cpha = cpha,
      .br_psc = br_psc,
  };

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

  PRINTD("Slave configuration terminated [fclk_hz = %u] [id = %u]", fclk_hz,
         id);

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

  LL_SPI_SetDataWidth(
      _spi, slave.nbit <= 8 ? LL_SPI_DATAWIDTH_8BIT : LL_SPI_DATAWIDTH_16BIT);
  LL_SPI_SetClockPolarity(
      _spi, slave.cpol ? LL_SPI_POLARITY_HIGH : LL_SPI_POLARITY_LOW);
  LL_SPI_SetClockPhase(_spi,
                       slave.cpha ? LL_SPI_PHASE_2EDGE : LL_SPI_PHASE_1EDGE);
  LL_SPI_SetBaudRatePrescaler(_spi, slave.br_psc << SPI_CR1_BR_Pos);

  _active_cfg = sid;
}

template <typename HwAlarm, size_t NMAX_SLAVES>
std::optional<typename SpiMaster<HwAlarm, NMAX_SLAVES>::DataT>
SpiMaster<HwAlarm, NMAX_SLAVES>::txrx(SlaveId sid, DataT txd,
                                      HwAlarmType::NanoSeconds t_pre,
                                      HwAlarmType::NanoSeconds t_post) {
  if (!_isValid(sid)) return {};

  const auto &slave = _slaves[sid];

  /* Configure SPI only if the slave has changed */
  if (_active_cfg != sid) _applyCfg(sid);

  /* Select 8bit/16bit frame format */
  const auto nbit_frame = slave.nbit <= 8 ? 8 : 16;

  /* !! Align data (slave.nbit wide) to the MSB of the frame (8/16 bit wide) */
  txd <<= nbit_frame - slave.nbit;

  /* 1) Select */
  LL_GPIO_ResetOutputPin(slave.ssn.gpio, slave.ssn.pin);
  _hw_alarm.delay(t_pre);  // Blocking operation

  /* 2) TX <---> RX */
  LL_SPI_Enable(_spi);
  if (nbit_frame == 8)
    LL_SPI_TransmitData8(_spi, static_cast<uint8_t>(txd));
  else
    LL_SPI_TransmitData16(_spi, txd);

  while (!LL_SPI_IsActiveFlag_RXNE(_spi));
  auto rply{nbit_frame == 8 ? static_cast<DataT>(LL_SPI_ReceiveData8(_spi))
                            : LL_SPI_ReceiveData16(_spi)};

  // TXE already 1
  while (LL_SPI_IsActiveFlag_BSY(_spi));
  LL_SPI_Disable(_spi);

  /* 3) Deselect */
  _hw_alarm.delay(t_post);  // Blocking operation
  LL_GPIO_SetOutputPin(slave.ssn.gpio, slave.ssn.pin);

  /* !! Align data (slave.nbit wide) to the LSB of the frame (8/16 bit wide) */
  rply >>= nbit_frame - slave.nbit;
  PRINTD("Master [0x%0*X] <---> [0x%0*X] Slave", nbit_frame/4, txd, nbit_frame/4, rply);

  return {rply};
}

#endif  // SPIMASTER_TPP