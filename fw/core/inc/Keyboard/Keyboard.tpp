/**
 * @file     Keyboard.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     17.07.2025
 */

#ifndef KEYBOARD_TPP
#define KEYBOARD_TPP

#include "poll.h"

template <typename SpiMaster, typename HwAlarm>
Keyboard<SpiMaster, HwAlarm>::Keyboard(SpiMaster& spi, HwAlarm& hw_alarm,
                                       Layout layout)
    : _spi(spi),
      _hw_alarm(hw_alarm),
      _ssn{},
      _cpol{},
      _cpha{},
      _max_fclk_hz{},
      _parser(layout),
      _state{},
      _peek{} {}

template <typename SpiMaster, typename HwAlarm>
void Keyboard<SpiMaster, HwAlarm>::setSsn(GPIO_TypeDef* gpio, uint32_t pin) {
  _ssn.gpio = gpio;
  _ssn.pin = pin;
}

template <typename SpiMaster, typename HwAlarm>
void Keyboard<SpiMaster, HwAlarm>::setMaxClockFreq(
    typename SpiMaster::ClockFreq fclk_hz) {
  _max_fclk_hz = fclk_hz;
}

template <typename SpiMaster, typename HwAlarm>
void Keyboard<SpiMaster, HwAlarm>::setOptions(bool cpol, bool cpha) {
  _cpol = cpol;
  _cpha = cpha;
}

template <typename SpiMaster, typename HwAlarm>
bool Keyboard<SpiMaster, HwAlarm>::_pollTxRx(FrameT sdo, FrameT& sdi,
                                             FrameT sdi_mask, bool active_level,
                                             size_t nmax_poll_cycles) const {
  auto rply = _spi.txrx(_id, sdo, T_SYNC, T_SYNC);
  if (!rply) return false;

  while (nmax_poll_cycles--) {
    _hw_alarm.delay(T_POLL);
    rply = _spi.txrx(_id, 0, T_SYNC, T_SYNC);
    if (!rply) return false;
    if (static_cast<bool>(*rply & sdi_mask) == active_level) {
      sdi = *rply;
      _hw_alarm.delay(T_GAP);
      return true;
    }
  }
  return false;
}

template <typename SpiMaster, typename HwAlarm>
bool Keyboard<SpiMaster, HwAlarm>::_trySet(bool caps_lock,
                                           bool num_lock) const {
  constexpr FrameT ACK_CODE = 0xFA;
  constexpr FrameT CMD_CODE = 0xED;
  const auto CMD_ARG = (static_cast<FrameT>(num_lock) << 1) |
                       (static_cast<FrameT>(caps_lock) << 2);

  FrameT sdi;

  if (!_pollTxRx(MOSI_SEND | (CMD_CODE << 8), sdi, MISO_TXC)) return false;
  PRINTD("[System] -- 0x%02X --> [Keyboard]", CMD_CODE);

  if (!_pollTxRx(MOSI_WEN | MOSI_CEN, sdi, MISO_RXV)) return false;
  PRINTD("[System] <-- 0x%02X -- [Keyboard]", (sdi >> 8));

  if (!_pollTxRx(MOSI_SEND | (CMD_ARG << 8), sdi, MISO_TXC)) return false;
  PRINTD("[System] -- 0x%02X --> [Keyboard]", CMD_ARG);

  if (!_pollTxRx(MOSI_WEN | MOSI_CEN, sdi, MISO_RXV)) return false;
  PRINTD("[System] <-- 0x%02X -- [Keyboard]", (sdi >> 8));

  return (sdi >> 8) == ACK_CODE;
}

template <typename SpiMaster, typename HwAlarm>
int Keyboard<SpiMaster, HwAlarm>::open(OFile& ofile) {
  if (ofile.mode != FREAD) return -EINVAL;

  /* Check configuration options were set */
  if (!_ssn.gpio || !_max_fclk_hz) return -ENOTSUP;

  /* Try to register with the master */
  auto id = _spi.addSlave(_ssn, std::numeric_limits<FrameT>::digits, _cpol,
                          _cpha, _max_fclk_hz);
  if (!id) return -ENOMEM;
  _id = *id;

  /* Issue a reset for the PS/2 controller and its FIFO */
  if (FrameT sdi; !_pollTxRx(MOSI_RST, sdi, MISO_EN)) return -EIO;
  PRINTD("PS/2 Controller and FIFO reset");

  /* Reset the state */
  if (!_trySet(false, false)) return -EIO;
  PRINTE("Cleared LED state");

  _state = {};

  return 0;
}

template <typename SpiMaster, typename HwAlarm>
int Keyboard<SpiMaster, HwAlarm>::_tryRead(uint8_t& scan_code) const {
  auto rply = _spi.txrx(_id, 0, T_SYNC, T_SYNC);
  if (!rply) return -EIO;

  const FrameT sdo = *rply;

  const bool has_data = sdo & MISO_RXV;
  if (has_data) scan_code = sdo >> 8;

  /* Prepare next read attempt */
  const auto enabled = sdo & MISO_EN;
  const auto ps2_rx_err = sdo & (MISO_FE | MISO_PE | MISO_CTO);
  if (!enabled || ps2_rx_err) {
    PRINTD(
        "Peripheral stalled. Resetting... "
        "[en = %u, fe = %u, pe = %u, cto = %u]",
        enabled, (sdo & MISO_FE), (sdo & MISO_PE), (sdo & MISO_CTO));

    _spi.txrx(_id, MOSI_RST, T_SYNC, T_SYNC);
    _hw_alarm.delay(T_GAP);
  }

  if (sdo & MISO_OE) PRINTD("Overrun detected");

  return has_data;
}

template <typename SpiMaster, typename HwAlarm>
bool Keyboard<SpiMaster, HwAlarm>::_getCapsLock() const {
  return (_state.caps_lock >> 1) ^ (_state.caps_lock & 1);
}

template <typename SpiMaster, typename HwAlarm>
bool Keyboard<SpiMaster, HwAlarm>::_getNumLock() const {
  return (_state.num_lock >> 1) ^ (_state.num_lock & 1);
}

template <typename SpiMaster, typename HwAlarm>
char Keyboard<SpiMaster, HwAlarm>::_map(UniKey k) const {
  const auto shift = _state.lshift || _state.rshift;
  const auto caps_lock = _getCapsLock();
  const auto num_lock = _getNumLock();

  const auto idx = (_state.alt_gr << 1) | shift;
  const auto idx_caps = idx ^ caps_lock;

  const auto ascii_lut = AsciiLut()[static_cast<uint8_t>(k)];
  auto c = ascii_lut[idx];

  if ((UniKey::KP_7 <= k && k <= UniKey::KP_ENTER) || k == UniKey::KP_EQUAL)
    return !isdigit(c) || num_lock ? c : 0;

  if (isalpha(c)) c = ascii_lut[idx_caps];

  return c;
}

template <typename SpiMaster, typename HwAlarm>
char Keyboard<SpiMaster, HwAlarm>::_step(uint8_t scan_code) {
  char mapped_key{};
  const auto [action, uni_key] = _parser.parse(scan_code);

  if (action == ScanCodeParser::RESET)
    _state = {};
  else if (action != ScanCodeParser::NONE) {
    const auto is_make = (action == ScanCodeParser::MAKE);
    /*         is_break     = (action == ScanCodeParser::BREAK)
     *                      = !is_make                          */

    switch (uni_key) {
      case UniKey::KB_CAPSLOCK:
        if (!(_state.caps_lock & 1) && is_make) {
          _state.caps_lock =
              KeyState{static_cast<uint8_t>(1 + _state.caps_lock)};
          if (!_trySet(_getCapsLock(), _getNumLock()))
            PRINTE("Failed to set LED state");
        } else if ((_state.caps_lock & 1) && !is_make) {
          _state.caps_lock = KeyState{
              static_cast<uint8_t>((_state.caps_lock + 1) & (NKEYSTATES - 1))};
        }
        break;

      case UniKey::KB_NUMLOCK:
        if (!(_state.num_lock & 1) && is_make) {
          _state.num_lock = KeyState{static_cast<uint8_t>(1 + _state.num_lock)};
          if (!_trySet(_getCapsLock(), _getNumLock()))
            PRINTE("Failed to set LED state");
        } else if ((_state.num_lock & 1) && !is_make) {
          _state.num_lock = KeyState{
              static_cast<uint8_t>((_state.num_lock + 1) & (NKEYSTATES - 1))};
        }
        break;

      /* Just active/inactive on make/break */
      case UniKey::KB_LSHIFT:
        _state.lshift = is_make;
        break;
      case UniKey::KB_RSHIFT:
        _state.rshift = is_make;
        break;
      case UniKey::KB_RALT:
        _state.alt_gr = is_make;
        break;

      /* Only active on make */
      default:
        if (is_make) mapped_key = _map(uni_key);
    }
  }

  return mapped_key;
}

template <typename SpiMaster, typename HwAlarm>
__poll_t Keyboard<SpiMaster, HwAlarm>::poll(OFile& ofile) {
  constexpr auto READY_MASK = POLLIN | POLLRDNORM;

  if (_peek) return READY_MASK;

  uint8_t scan_code;
  const auto ret = _tryRead(scan_code);
  if (ret < 0) return POLLERR;
  if (!ret) return 0;

  return ((_peek = _step(scan_code))) ? READY_MASK : 0;
}

template <typename SpiMaster, typename HwAlarm>
int Keyboard<SpiMaster, HwAlarm>::_getc(char& c) {
  uint8_t scan_code;
  char cc{};

  do {
    const auto ret = _tryRead(scan_code);
    if (ret > 0)
      cc = _step(scan_code);
    else if (ret < 0)
      return ret;

    _hw_alarm.delay(T_POLL);
  } while (!cc);

  c = cc;
  return 1;
}

template <typename SpiMaster, typename HwAlarm>
ssize_t Keyboard<SpiMaster, HwAlarm>::read(OFile& ofile, char* buf,
                                           size_t count, off_t& pos) {
  if (!count) return 0;

  if (_peek) {
    *buf = _peek;
    _peek = 0;
    return 1;
  }

  if (ofile.flags & O_NONBLOCK) return -EAGAIN;

  return _getc(*buf);
}

#endif  // KEYBOARD_TPP
