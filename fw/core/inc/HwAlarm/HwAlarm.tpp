/**
 * @file     HwAlarm.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     24.01.2025
 */

#ifndef HWALARM_TPP
#define HWALARM_TPP

#include <debug.h>
#include <stm32f4xx_ll_tim.h>

#include <algorithm>

template <uintptr_t TimBase>
HwAlarm<TimBase>::HwAlarm()
    : _alarms{},
      _psc_clk(0),
      _psc_plus_one_times_den(0),
      _psc_plus_one_times_half_den(0) {}

template <uintptr_t TimBase>
bool HwAlarm<TimBase>::calcTimeBase(const NanoSeconds &t_cnt, Psc &psc) {
  constexpr auto psc_width = std::numeric_limits<Psc>::digits;
  constexpr auto ratio_den = static_cast<DurationRep>(NanoSeconds::period::den);

  const auto psc_clk = tim::getPscClock(TimBase);
  const auto ticks = t_cnt.count() * psc_clk;

  if (ticks < ratio_den || ticks > (ratio_den << psc_width)) return false;

  /* Floor gives at least target resolution, or better */
  auto psc_plus_one = ticks / ratio_den;

  _psc_clk = psc_clk;
  _psc_plus_one_times_den = psc_plus_one * ratio_den;
  _psc_plus_one_times_half_den = psc_plus_one * (ratio_den >> 1);
  psc = psc_plus_one - 1;

  return true;
}

template <uintptr_t TimBase>
bool HwAlarm<TimBase>::setResolution(const NanoSeconds &t_cnt) {
  Psc psc;
  if (!calcTimeBase(t_cnt, psc)) return false;

  LL_TIM_SetPrescaler(_tim, psc);
  LL_TIM_GenerateEvent_UPDATE(_tim);
  return true;
}

template <uintptr_t TimBase>
bool HwAlarm<TimBase>::init(const NanoSeconds &t_cnt, uint32_t preempt,
                            uint32_t sub) {
  tim::enableClock(TimBase);

  /*
   * ARR not preloaded
   * If mode is supported: edge-aligned, upcounting
   * Not stopped at UEV
   * UEV enabled: UG not triggering UEV
   */
  _tim->CR1 = TIM_CR1_URS;

  /* Free running timer */
  LL_TIM_SetAutoReload(_tim, std::numeric_limits<Cnt>::max());

  /* Set PSC, generates UEV (reset PSC and CNT) */
  if (!setResolution(t_cnt)) return false;

  /*
   * CCMR @ reset
   *  - Output channel
   *  - CCR not preloaded
   *  - Frozen mode
   */

  NVIC_SetPriority(
      tim::getIRQn(TimBase),
      NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt, sub));
  NVIC_EnableIRQ(tim::getIRQn(TimBase));

  LL_TIM_EnableCounter(_tim);
  return true;
}

template <uintptr_t TimBase>
size_t HwAlarm<TimBase>::getChannel() const {
  size_t idx;
  for (idx = 0; idx < _alarms.size(); ++idx) {
    if (!_alarms[idx].icb) return idx;
  }
  return idx;
}

template <uintptr_t TimBase>
void HwAlarm<TimBase>::freeChannel(size_t ch) {
  _alarms[ch].icb = nullptr;

  /* disable IRQ and re-evaluate all */
  tim::disableItCC(_tim, ch);
  NVIC_ClearPendingIRQ(tim::getIRQn(TimBase));
}

template <uintptr_t TimBase>
auto HwAlarm<TimBase>::setAlarm(const NanoSeconds &delay,
                                const ICallbackType *icb, uint32_t reps)
    -> AlarmState {
  /* save time of request asap */
  const auto cnt = static_cast<Cnt>(_tim->CNT);

  /* exit early if parameters are invalid */
  if (!icb || !*icb) return INVALID_CALLBACK;

  /* exit early if no resources are free */
  const auto idx = getChannel();
  if (idx == _alarms.size()) return CHANNELS_BUSY;

  const auto ticks_wide =
      ((delay.count() * _psc_clk) + _psc_plus_one_times_half_den) /
      _psc_plus_one_times_den;
  const auto ticks = static_cast<Cnt>(ticks_wide);

  if (!ticks_wide || ticks_wide > std::numeric_limits<Cnt>::max())
    return INVALID_DELAY;

  /* set alarm representation */
  _alarms[idx].reps = !reps ? std::numeric_limits<uint32_t>::max() : reps;
  _alarms[idx].ticks = ticks;

  /* set channel */
  *tim::ccr<TimBase>(idx) = cnt + ticks;
  tim::clearFlagCC(_tim, idx);

  /* lock and start */
  NVIC_DisableIRQ(tim::getIRQn(TimBase));
  if (static_cast<Cnt>(_tim->CNT) - cnt > ticks) { /* delay already elapsed */
    NVIC_EnableIRQ(tim::getIRQn(TimBase));
    return DELAY_TOO_SHORT;
  }
  /* mark channel in use and activate */
  _alarms[idx].icb = icb;
  tim::enableItCC(_tim, idx);

  /* re-evaluate channels' IRQs */
  NVIC_ClearPendingIRQ(tim::getIRQn(TimBase));
  NVIC_EnableIRQ(tim::getIRQn(TimBase));
  return STARTED;
}

template <uintptr_t TimBase>
auto HwAlarm<TimBase>::setAlarm(const ICallbackType *icb, uint32_t reps,
                                const NanoSeconds &delay,
                                const ICallbackType *icb_new) -> AlarmState {
  if (!icb || (icb_new && !*icb_new)) return INVALID_CALLBACK;

  /* lock alarm representation */
  NVIC_DisableIRQ(tim::getIRQn(TimBase));

  /* Find alarm to modify */
  size_t idx = 0;
  for (; idx < _alarms.size() && _alarms[idx].icb != icb; ++idx);
  if (idx == _alarms.size()) {
    NVIC_EnableIRQ(tim::getIRQn(TimBase));
    return INVALID_CALLBACK;
  }

  /* Modify repetitions */
  if (!reps) {
    freeChannel(idx);
    NVIC_EnableIRQ(tim::getIRQn(TimBase));
    return STOPPED;
  }

  /* Modify alarm representation */
  _alarms[idx].reps = reps;

  if (delay != NanoSeconds::zero()) {
    /* recover original time instant */
    auto cnt = *tim::ccr<TimBase>(idx) - _alarms[idx].ticks;

    /* recalculate timing parameters */
    const auto ticks_wide =
        ((delay.count() * _psc_clk) + _psc_plus_one_times_half_den) /
        _psc_plus_one_times_den;
    _alarms[idx].ticks = static_cast<Cnt>(ticks_wide);

    if (!ticks_wide || ticks_wide > std::numeric_limits<Cnt>::max()) {
      freeChannel(idx);
      NVIC_EnableIRQ(tim::getIRQn(TimBase));
      return INVALID_DELAY;
    }

    /* set channel */
    *tim::ccr<TimBase>(idx) = cnt + _alarms[idx].ticks;
    tim::clearFlagCC(_tim, idx);

    /* delay already elapsed ? */
    if (static_cast<Cnt>(_tim->CNT) - cnt > _alarms[idx].ticks) {
      freeChannel(idx);
      NVIC_EnableIRQ(tim::getIRQn(TimBase));
      return DELAY_TOO_SHORT;
    }
  }

  /* Modify callback */
  if (icb_new) _alarms[idx].icb = icb_new;

  /* re-evaluate channels' IRQs */
  NVIC_ClearPendingIRQ(tim::getIRQn(TimBase));
  NVIC_EnableIRQ(tim::getIRQn(TimBase));
  return CHANGED;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wvolatile"
/* compound assignment with 'volatile'-qualified left operand is deprecated */

template <uintptr_t TimBase>
void HwAlarm<TimBase>::handler() {
  constexpr auto max_reps = std::numeric_limits<uint32_t>::max();
  std::array<const ICallbackType *, _nch> scheduled{};

  /* one channel after the other */
  for (size_t idx = 0; idx < _alarms.size(); ++idx) {
    /* if in use && has triggered */
    if (_alarms[idx].icb && tim::isActiveFlagCC(_tim, idx)) {
      /* schedule its execution */
      scheduled[idx] = _alarms[idx].icb;

      /* done ? clear channel */
      if (_alarms[idx].reps != max_reps && 0 == --_alarms[idx].reps) {
        tim::disableItCC(_tim, idx);
        _alarms[idx].icb = nullptr;
      }
      /* rearm ? update ccr */
      else {
        *tim::ccr<TimBase>(idx) += _alarms[idx].ticks;
        tim::clearFlagCC(_tim, idx);
      }
    }
  }
  /* re-evaluate channels' IRQs */
  NVIC_ClearPendingIRQ(tim::getIRQn(TimBase));

  /* delayed execution */
  for (const auto icb : scheduled) {
    if (icb) (*icb)();
  }
}
#pragma GCC diagnostic pop

template <uintptr_t TimBase>
typename HwAlarm<TimBase>::Cnt HwAlarm<TimBase>::now() const {
  return _tim->CNT;
}

template <uintptr_t TimBase>
bool HwAlarm<TimBase>::getTick(const NanoSeconds &delay, Cnt &cticks) const {
  /* save time of request asap */
  const auto cnt = static_cast<Cnt>(_tim->CNT);

  const auto ticks_wide =
      ((delay.count() * _psc_clk) + _psc_plus_one_times_half_den) /
      _psc_plus_one_times_den;
  const auto ticks = static_cast<Cnt>(ticks_wide);

  if (!ticks_wide || ticks_wide > std::numeric_limits<Cnt>::max()) return false;

  cticks = cnt + ticks;
  return true;
}

#endif  // HWALARM_TPP
