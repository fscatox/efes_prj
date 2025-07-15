/**
 * @file     SSegDisplay.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     29.01.2025
 */

#ifndef SSEGDISPLAY_TPP
#define SSEGDISPLAY_TPP

#include <cctype>

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
template <size_t... SEG_IDX_OFF>
consteval auto SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::fmtChar()
    -> CharType {
  constexpr auto CharSize = std::numeric_limits<CharType>::digits;
  static_assert(sizeof...(SEG_IDX_OFF) < CharSize, "Too many indexes");
  static_assert(((SEG_IDX_OFF < CharSize - 1) && ...), "Index out of range");

  if constexpr (sizeof...(SEG_IDX_OFF) == 0) {
    /* All segments on */
    return SEG_ON_HIGH ? 0x7F : 0x00;
  } else {
    /* Some segments off */
    constexpr auto off_mask = (... | (0x01 << SEG_IDX_OFF));
    return SEG_ON_HIGH ? ~off_mask : off_mask;
  }
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
constexpr auto SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::encode(CharType c)
    -> CharType {

  static CharType digits[]{_0, _1, _2, _3, _4, _5, _6, _7, _8, _9};
  static CharType alpha[]{
      A, B, C, D,      E, F,      NOTSUP, H, I,      J,      NOTSUP, L, NOTSUP,
      N, O, P, NOTSUP, R, NOTSUP, T,      U, NOTSUP, NOTSUP, NOTSUP, Y, NOTSUP};

  if (isdigit(c))
    return digits[c - '0'];

  if (isalpha(c))
    return alpha[toupper(c) - 'A'];

  if (isblank(c))
    return SPACE;

  if (isspace(c))
    return CLEAR;

  switch (c) {
    case '|':
      return SEP_V;

    case '(':
    case '[':
    case '{':
      return BRACK_L;

    case ')':
    case ']':
    case '}':
      return BRACK_R;

    case '-':
      return DASH;

    case '.':
    case ',':
    case '_':
      return DOT;

    default:
      return NOTSUP;
  }
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::SSegDisplay(USART_TypeDef *usart,
                                                         DMA_TypeDef *dma,
                                                         HwAlarm &hw_alarm)
    : UartTx<BUF_SIZE>(usart, dma), _state(BUILD_STRING),
      _buf_end(this->_buf.begin()), _hw_alarm(hw_alarm),
      _alarm_cb(this, &SSegDisplay::alarm) {}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
void SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::setDisplay(
    size_t len, MilliSeconds scroll_delay) {
  _scroll_delay = scroll_delay;
  _disp_len = len;
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
int SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::open(OFile &ofile) {
  /* Initialize GPIO, DMA, USART */
  if (int ret; (ret = UartTx<BUF_SIZE>::open(ofile)) < 0)
    return ret;

  /* Reset peripheral? */
  if (ofile.flags & FTRUNC)
    LL_USART_TransmitData8(this->_usart, CLEAR);

  return 0;
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
int SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::close(OFile &ofile) {
  if (_state == TRANSFER_SCROLL) {
    /* Wait to have scrolled at least once */
    while (!_scrolled_once);
    /* Kill scroll task */
    _hw_alarm.setAlarm(&_alarm_cb, 0);
  }
  _state = BUILD_STRING;
  return UartTx<BUF_SIZE>::close(ofile);
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
bool SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::wait(bool non_block) {

  if (_state == TRANSFER_SCROLL) {
    /* Wait to have scrolled at least once */
    while (!_scrolled_once) {
      if (non_block)
        return false;
    }
    /* Kill scroll task */
    _hw_alarm.setAlarm(&_alarm_cb, 0);
    /* Fallthrough NO_SCROLL*/
    _state = TRANSFER_NO_SCROLL;
  }

  if (_state == TRANSFER_NO_SCROLL) {
    /* Wait for ongoing transfer to complete */
    if (!UartTx<BUF_SIZE>::wait(non_block))
      return false;

    /* Clear buffer */
    _buf_end = this->_buf.begin();
    /* Fallthrough BUILD_STRING */
    _state = BUILD_STRING;
  }

  return true;
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
ssize_t SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::write(OFile &ofile,
                                                           const char *buf,
                                                           size_t count,
                                                           off_t &pos) {

  if (_state != BUILD_STRING) {
    if (!wait(ofile.flags & FNONBLOCK))
      return -EAGAIN;
    pos = 0;
  }

  /* Find first terminated substring */
  const auto buf_end = buf + count;
  auto it = std::find(buf, buf_end, _end_char);

  /* If present, strip terminator */
  const auto strip_count = std::distance(buf, it);

  /* Check if there is enough space left for concatenation
   * (one char is always reserved for a spacer) */
  const auto buf_count = std::distance(this->_buf.begin(), _buf_end);
  if (static_cast<size_t>(buf_count) + strip_count > this->_buf.size() - 1)
    return -ENOSPC;

  /* And remap characters with 7seg encoding */
  _buf_end = std::transform(buf, it, _buf_end, encode);

  /* Terminated? Transfer the whole string to the display */
  if (it != buf_end) {
    auto str_len =
        static_cast<size_t>(std::distance(this->_buf.begin(), _buf_end));

    if (str_len <= _disp_len) { /* It fits, it's a single transfer */
      this->startDMATransfer(this->_buf.begin(), str_len);
      _state = TRANSFER_NO_SCROLL;
    } else { /* It requires scrolling (head & tail separated with a space) */
      *(_buf_end++) = encode(' ');
      _scroll_ptr = this->_buf.begin() + 1;
      _scrolled_once = false;
      this->startDMATransfer(this->_buf.begin(), _disp_len);
      _state = TRANSFER_SCROLL;
      _hw_alarm.setAlarm(_scroll_delay, &_alarm_cb, 0);
    }
  }

  pos += strip_count;
  return strip_count + (it != buf_end);
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
off_t SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::llseek(OFile &ofile,
                                                          off_t offset,
                                                          int whence) {

  const off_t new_pos = offset + (whence == SEEK_CUR ? ofile.pos : 0);

  /* If transmitting, allowed to clear the display by seeking to zero
   * If building a string, allowed to seek back */
  if ((_state != BUILD_STRING && new_pos != 0) ||
      (_state == BUILD_STRING &&
       new_pos >= std::distance(this->_buf.begin(), _buf_end)))
    return -EINVAL;

  if (_state == BUILD_STRING)
    _buf_end = this->_buf.begin() + new_pos;
  else {
    /* wait for the transmission to end */
    if (!wait(ofile.flags & FNONBLOCK))
      return -EAGAIN;
    /* clear the display */
    LL_USART_TransmitData8(this->_usart, CLEAR);
    _state = TRANSFER_NO_SCROLL;
  }

  ofile.pos = new_pos;
  return new_pos;
}

template <typename HwAlarm, size_t BUF_SIZE, bool SEG_ON_HIGH>
void SSegDisplay<HwAlarm, BUF_SIZE, SEG_ON_HIGH>::alarm() {
  if (this->isSending())
    exit(-3); /* scroll time too short? Something very weird is happening */

  /* Last contiguous transfer?
   * Shift [_scroll_ptr, _buf_end] to begin */
  if (_scroll_ptr + _disp_len == _buf_end) {
    std::rotate(this->_buf.begin(), _scroll_ptr, _buf_end);
    _scroll_ptr = this->_buf.begin();
    _scrolled_once = true;
  }

  this->startDMATransfer(_scroll_ptr, _disp_len);
  _scroll_ptr = std::next(_scroll_ptr);
}

#endif // SSEGDISPLAY_TPP
