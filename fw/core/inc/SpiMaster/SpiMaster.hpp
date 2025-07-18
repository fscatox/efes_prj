/**
 * @file     SpiMaster.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     15.07.2025
 */

#ifndef SPIMASTER_HPP
#define SPIMASTER_HPP

#include <array>
#include <optional>

#include "gpio.h"
#include "spi.h"

template <typename HwAlarm, size_t NMAX_SLAVES = 4>
class SpiMaster {
 public:
  using SlaveId = size_t;
  using ClockFreq = uint32_t;
  using FrameT = uint16_t;

  struct PinCfg {
    GPIO_TypeDef *gpio;
    uint32_t pin;
  };
  struct AltPinCfg {
    PinCfg pin_cfg;
    uint32_t af;
  };

  SpiMaster(SPI_TypeDef *spi, HwAlarm &hw_alarm);
  void init(const AltPinCfg &miso, const AltPinCfg &mosi,
            const AltPinCfg &sclk);

  std::optional<SlaveId> addSlave(const PinCfg &ssn, size_t nbits, bool cpol,
                                  bool cpha, ClockFreq max_fclk_hz);
  void removeSlave(SlaveId sid);

  std::optional<FrameT> txrx(SlaveId sid, FrameT txd,
                             HwAlarmType::NanoSeconds t_pre = HwAlarmType::NanoSeconds{30},
                             HwAlarmType::NanoSeconds t_post = HwAlarmType::NanoSeconds{30});

 private:
  struct Slave {
    PinCfg ssn;
    size_t nbits;
    bool cpol, cpha;
    uint8_t br_psc;
  };
  using SlaveTable = std::array<Slave, NMAX_SLAVES>;

  constexpr static auto T_WEAK_PUPD = HwAlarmType::MicroSeconds{1};

  bool _isValid(SlaveId sid) const;
  void _applyCfg(SlaveId sid);

  SPI_TypeDef *_spi;
  HwAlarm &_hw_alarm;
  PinCfg _sclk;
  SlaveTable _slaves;
  SlaveId _active_cfg;
};

#include "SpiMaster.tpp"

#endif  // SPIMASTER_HPP
