/**
 * @file     LTC2308.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     16.07.2025
 */

#ifndef LTC2308_TPP
#define LTC2308_TPP

#include <limits>

#include "debug.h"

using namespace std::chrono_literals;

template <typename SpiMaster, typename HwAlarm>
LTC2308<SpiMaster, HwAlarm>::LTC2308(SpiMaster& spi, HwAlarm& hw_alarm)
    : _spi(spi),
      _hw_alarm(hw_alarm),
      _id(std::numeric_limits<decltype(_id)>::max()),
      _convst{},
      _max_fclk_hz{},
      _sdo(NULL_FRAME),
      _sdi(NULL_FRAME) {}

template <typename SpiMaster, typename HwAlarm>
void LTC2308<SpiMaster, HwAlarm>::setConvst(GPIO_TypeDef* gpio, uint32_t pin) {
  _convst = {.gpio = gpio, .pin = pin};
}

template <typename SpiMaster, typename HwAlarm>
bool LTC2308<SpiMaster, HwAlarm>::setMaxClockFreq(
    typename SpiMaster::ClockFreq fclk_hz) {
  if (fclk_hz > OPT_FCLK_MAX) return false;
  _max_fclk_hz = fclk_hz;
  return true;
}

template <typename SpiMaster, typename HwAlarm>
void LTC2308<SpiMaster, HwAlarm>::setOptions(InputCfg icfg, Channel ch_plus,
                                             RangeType type, bool low_power) {
  _sdo = icfg << MOSI_SD_POS | (ch_plus % 2) << MOSI_OS_POS |
          (ch_plus / 2) << MOSI_S0_POS | type << MOSI_UNI_POS;
  _lp = low_power;
}

template <typename SpiMaster, typename HwAlarm>
int LTC2308<SpiMaster, HwAlarm>::open(OFile& ofile) {
  if (ofile.mode != FREAD) return -EINVAL;

  /* Check configuration options were set */
  if (!_convst.gpio || !_max_fclk_hz || _sdo == NULL_FRAME) return -ENOTSUP;

  /* Try to register with the master */
  auto id =
      _spi.addSlave(_convst, FRAME_NBIT, OPT_CPOL, OPT_CPHA, _max_fclk_hz);
  if (!id) return -ENOMEM;
  _id = *id;

  /* The slave is now deselected.
   * Given that gpio::init() already forced CONVST up, nothing to do */

  /* Write configuration possibly entering sleep mode
   * (MOSI word aligned to MISO MSB */
  const auto sdo = _sdo | (static_cast<FrameT>(_lp) << MOSI_SLP_POS);
  _spi.txrx(_id, sdo << (FRAME_NBIT - CFG_NBIT), T_EN, T_DIS);

  return 0;
}

template <typename SpiMaster, typename HwAlarm>
ssize_t LTC2308<SpiMaster, HwAlarm>::read(OFile& ofile, char* buf, size_t count,
                                          off_t& pos) {
  if (!count) return 0;

  const auto rqst_count = count;
  PRINTD("Requested %u bytes", count);

  /* If the previous call asked for an odd number of bytes, there is one
   * leftover byte to transfer: given that sizeof(FrameT) == 2, it is known
   * that the byte left to transfer is ((char *)&_miso)[1].
   * Otherwise, the rule is to leave NULL_FRAME in the _miso spot. */
  if (_sdi != NULL_FRAME) {
    PRINTD("Transfer leftover data [_miso = 0x%02X]", _sdi);
    *buf++ = reinterpret_cast<char*>(&_sdi)[1];
    _sdi = NULL_FRAME;
    if (!--count) return 1;
  }

  /*         | last read()
   *         v         __________________________
   * CONVST  _________/ <-- t_conv -->| NAP/SLEEP
   *                  ^ conversion start
   */
  _hw_alarm.delay(T_CONV);

  if (_lp) {
    /* If in deep sleep, reconfigure in nap mode.
     *
     *                   | select to send cfg_word
     *                   |                | parsed
     *            _______v                v_______________________
     *    CONVST  SLEEP xx\_______________/
     *                                    <-- t_refwake to wake up
     */
    _spi.txrx(_id, _sdo << (FRAME_NBIT - CFG_NBIT), T_EN, T_DIS);
    _hw_alarm.delay(T_REFWAKE);
  }

  /*
   *         __________________________
   * CONVST  xx NAP xx
   *
   */

  /*
   * 1) Trigger a new conversion
   *    S&H samples, SPI transfer skipped
   *
   *                                                   | start conversion
   *            _____________                          v_______
   *    CONVST  xx NAP xx    \________________________/
   *                         |<-- > t_acq by sclk -->|
   */
  _spi.txrx(_id, _sdo << (FRAME_NBIT - CFG_NBIT), T_EN, T_DIS);

  /* The frame transferred with the peripheral is sizeof(FrameT) bytes wide.
   * Given count, round it up to the nearest even: if present, the extra byte
   * will be taken at the next call */
  for (auto ntransfers = (count + 1) >> 1; ntransfers > 0; --ntransfers) {
    /*
     * 2) Wait for the conversion to terminate
     *    After the conversion ends, the ADC enters NAP/SLEEP mode
     *    based on the configuration bit SLP
     *
     *                                                | latched data
     *                  ______________________________v
     *    CONVST  _____/               xx NAP/SLEEP xx\______
     *                 |<-- t_conv -->|
     */
    _hw_alarm.delay(T_CONV);

    /*
     * 3) Transfer data & start next conversion
     *    If it's the last conversion, enter deep sleep (provided it's enabled)
     *            _______________                  ___________________
     *    CONVST  xx NAP/SLEEP xx\________________/ xx NAP (SLEEP?) xx
     *
     */
    const auto sdo = _sdo | (static_cast<FrameT>(_lp && ntransfers == 1) << MOSI_SLP_POS);
    auto rply = _spi.txrx(_id, sdo << (FRAME_NBIT - CFG_NBIT), T_EN, T_DIS);
    if (!rply) return -EIO;

    _sdi = *rply;

    /* Apply sign extension if range is bipolar */
    if (!(_sdo & (0x1 << MOSI_UNI_POS))) {
      const FrameT sgn_msk = 0x1 << (FRAME_NBIT - 1);
      _sdi = (_sdi ^ sgn_msk) - sgn_msk;
      PRINTD("Bipolar range: frame was sign-extended [_miso = 0x%02X]", _sdi);
    }

    /* Copy to user buffer */
    size_t i = 0;
    for (; i < sizeof(FrameT) && count > 0; ++i, --count)
      *buf++ = reinterpret_cast<char*>(&_sdi)[i];

    /* Clear _miso if data was transferred entirely */
    if (i == sizeof(FrameT)) {
      PRINTD("Full transfer: _miso nulled [ntransfers = %u]", ntransfers);
      _sdi = NULL_FRAME;
    }
  }

  return rqst_count;
}

#endif  // LTC2308_TPP
