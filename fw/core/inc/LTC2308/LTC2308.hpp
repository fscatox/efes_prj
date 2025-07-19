/**
 * @file     LTC2308.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     16.07.2025
 */

#ifndef LTC2308_HPP
#define LTC2308_HPP

#include "IFile.h"

template <typename SpiMaster, typename HwAlarm>
class LTC2308 : public IFile {
 public:
  enum Channel : uint8_t { CH0 = 0, CH1, CH2, CH3, CH4, CH5, CH6, CH7 };
  enum InputCfg : uint8_t { DIFFERENTIAL = 0, SINGLE_ENDED };
  enum RangeType : uint8_t { BIPOLAR = 0, UNIPOLAR };

  LTC2308(SpiMaster& spi, HwAlarm& hw_alarm);

  void setConvst(GPIO_TypeDef* gpio, uint32_t pin);
  bool setMaxClockFreq(typename SpiMaster::ClockFreq fclk_hz);
  void setOptions(InputCfg icfg, Channel ch_plus, RangeType type,
                  bool low_power = false);

  int open(OFile& ofile) override;
  ssize_t read(OFile& ofile, char* buf, size_t count, off_t& pos) override;

 private:
  using FrameT = uint16_t;

  constexpr static auto MOSI_SLP_POS = 0; /*!< Sleep Mode */
  constexpr static auto MOSI_UNI_POS = 1; /*!< Unipolar/Bipolar_N */
  constexpr static auto MOSI_S0_POS = 2;  /*!< Address Select [0] */
  constexpr static auto MOSI_OS_POS = 4;  /*!< Odd/Sign_N */
  constexpr static auto MOSI_SD_POS = 5;  /*!< SE/Differential_N */

  constexpr static auto CFG_NBIT = 6;    /*!< 6-bit cfg word */
  constexpr static auto FRAME_NBIT = 12; /*!< 12-bit ADC */

  constexpr static auto OPT_CPOL = 0; /*!< Clock polarity */
  constexpr static auto OPT_CPHA = 0; /*!< Clock phase */
  constexpr static auto OPT_FCLK_MAX = [] {
    /* CONVST Low Time During Data Transfer */
    constexpr auto t_wlconvst_s = 410E-9;
    constexpr auto rx_cycles = std::numeric_limits<FrameT>::digits;
    return static_cast<typename SpiMaster::ClockFreq>(rx_cycles / t_wlconvst_s);
  }();

  constexpr static auto T_REFWAKE = typename HwAlarm::MilliSeconds{200};
  constexpr static auto T_EN = typename HwAlarm::NanoSeconds{30};
  constexpr static auto T_DIS = typename HwAlarm::NanoSeconds{30};
  constexpr static auto T_CONV = typename HwAlarm::NanoSeconds{1600};

  /* FrameT is wider than the actual frame width */
  constexpr static auto NULL_FRAME = std::numeric_limits<FrameT>::max();

  SpiMaster& _spi;
  HwAlarm& _hw_alarm;
  typename SpiMaster::SlaveId _id;
  typename SpiMaster::PinCfg _convst;
  typename SpiMaster::ClockFreq _max_fclk_hz;
  bool _lp;
  FrameT _sdo, _sdi;
};

#include "LTC2308.tpp"

#endif  // LTC2308_HPP
