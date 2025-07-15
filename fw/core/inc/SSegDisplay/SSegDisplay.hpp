/**
 * @file     SSegDisplay.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     29.01.2025
 */

#ifndef SSEGDISPLAY_HPP
#define SSEGDISPLAY_HPP

#include "CallbackUtils.hpp"
#include "UartTx.hpp"

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
class SSegDisplay final : public UartTx<BUF_SIZE> {
public:
  using CharType = uint8_t;
  using MilliSeconds = typename HwAlarm::MilliSeconds;

  SSegDisplay(USART_TypeDef *usart, DMA_TypeDef *dma, HwAlarm &hw_alarm);

  void setDisplay(size_t len, MilliSeconds scroll_delay = MilliSeconds{250});

  int open(OFile &ofile) override;
  int close(OFile &ofile) override;

  ssize_t write(OFile &ofile, const char *buf, size_t count,
                off_t &pos) override;
  off_t llseek(OFile &ofile, off_t offset, int whence) override;

private:

  /**
   * @brief Compile-time character encoding helper function
   *
   * Accept a list of non-type template arguments, representing the indexes of
   * the segments to turn off. The mapping is:
   *      0
   *     ---
   *  5 |   | 1
   *     ---  <-- 6
   *  4 |   | 2
   *     ---
   *      3
   *
   * @tparam SEG_IDX_OFF Non-type pack of indexes to turn off, 0 to 6
   * @return Character encoding
   */
  template <size_t... SEG_IDX_OFF>
  static consteval CharType fmtChar();
  static constexpr CharType encode(CharType c);

  using CallbackType =
      MemFnCallback<SSegDisplay, typename HwAlarm::ICallbackType::FnType>;
  using BufferType = typename UartTx<BUF_SIZE>::BufferType;

  enum State { BUILD_STRING, TRANSFER_NO_SCROLL, TRANSFER_SCROLL };
  enum Alphabet : CharType {
    SPACE = fmtChar<0, 1, 2, 3, 4, 5, 6>(),
    CLEAR = 0x80,
    SEP_V = fmtChar<0, 3, 6>(),
    BRACK_L = fmtChar<1, 2, 6>(),
    BRACK_R = fmtChar<4, 5, 6>(),
    DASH = fmtChar<0, 1, 2, 3, 4, 5>(),
    DOT = fmtChar<0, 1, 2, 4, 5, 6>(),
    NOTSUP = fmtChar<1, 2, 4, 5>(),
    _0 = fmtChar<6>(),
    _1 = fmtChar<0, 3, 4, 5, 6>(),
    _2 = fmtChar<2, 5>(),
    _3 = fmtChar<4, 5>(),
    _4 = fmtChar<0, 3, 4>(),
    _5 = fmtChar<1, 4>(),
    _6 = fmtChar<1>(),
    _7 = fmtChar<3, 4, 5, 6>(),
    _8 = fmtChar(),
    _9 = fmtChar<4>(),
    A = fmtChar<3>(),
    B = fmtChar<0, 1>(),
    C = fmtChar<0, 1, 2, 5>(),
    D = fmtChar<0, 5>(),
    E = fmtChar<1, 2>(),
    F = fmtChar<1, 2, 3>(),
    H = fmtChar<0, 1, 3>(),
    I = fmtChar<0, 1, 2, 3, 6>(),
    J = fmtChar<0, 4, 5, 6>(),
    L = fmtChar<0, 1, 2, 6>(),
    N = fmtChar<0, 1, 3, 5>(),
    O = fmtChar<0, 1, 5>(),
    P = fmtChar<2, 3>(),
    R = fmtChar<0, 1, 2, 3, 5>(),
    T = fmtChar<0, 1, 2>(),
    U = fmtChar<0, 1, 5, 6>(),
    Y = fmtChar<0, 4>(),
  };

  bool wait(bool non_block = false);
  void alarm();

  static constexpr CharType _end_char{'\n'};

  volatile State _state;
  typename BufferType::iterator _buf_end;
  HwAlarm &_hw_alarm;
  CallbackType _alarm_cb;
  MilliSeconds _scroll_delay;
  size_t _disp_len;
  volatile bool _scrolled_once;
  volatile typename BufferType::iterator _scroll_ptr;
};

#include "SSegDisplay.tpp"

#endif // SSEGDISPLAY_HPP
