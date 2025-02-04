/**
 * @file     BStepper.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     30.01.2025
 */

#include "BStepper.h"

#include <debug.h>
#include <numeric>

BStepper::BStepper(GPIO_TypeDef *gpio, TIM_TypeDef *tim, DMA_TypeDef *dma)
    : _gpio(gpio), _tim(tim), _dma(dma) {}

void BStepper::setPins(const Pinout &p) {
  _pins = p;
  _tr.setPins(_pins.ph);
}

void BStepper::setDMATransfer(uint32_t stream, uint32_t ch,
                              uint32_t stream_priority) {
  _dma_stream = stream;
  _dma_channel = ch;
  _dma_priority = stream_priority;
}

void BStepper::setResolution(uint16_t steps_per_rev) {
  _steps_per_rev = steps_per_rev;
}

uint16_t BStepper::getResolution() const {
  return _steps_per_rev;
}

void BStepper::updateClock() {
  /* Scaled prescaler clock (milli_rev_per_minute to rev_per_second) */
  const auto psc_clk_hz = tim::getPscClock(_tim);
  _sixtyk_psc_clk_hz = static_cast<uint64_t>(psc_clk_hz) * 60 * 1000;
}

void BStepper::init(uint32_t preempt, uint32_t sub) {

  /* Initialize GPIO peripheral */
  gpio::enableClock(_gpio);

  LL_GPIO_InitTypeDef gpio_init{
      .Pin = _pins.en | _pins.ph.a.pos | _pins.ph.a.neg | _pins.ph.b.pos |
             _pins.ph.b.neg,
      .Mode = LL_GPIO_MODE_OUTPUT,
      .Speed = LL_GPIO_SPEED_FREQ_LOW,
      .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
      .Pull = LL_GPIO_PULL_NO
  };
  LL_GPIO_Init(_gpio, &gpio_init);

  /* Drive to home step (still disabled) */
  disable();
  _gpio->BSRR = _tr.setHome();

  /* Initialize DMA peripheral */
  dma::enableClock(_dma);
  LL_DMA_InitTypeDef dma_init{
      .PeriphOrM2MSrcAddress = reinterpret_cast<uintptr_t>(&_gpio->BSRR),
      .Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
      .Mode = LL_DMA_MODE_CIRCULAR,
      .PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT,
      .MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT,
      .PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD,
      .MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD,
      .Channel = _dma_channel,
      .Priority = _dma_priority,
      .FIFOMode = LL_DMA_FIFOMODE_DISABLE
  };
  LL_DMA_Init(_dma, _dma_stream, &dma_init);

  /* Initialize TIM peripheral */
  tim::enableClock(_tim);
  updateClock();

  /*
   * ARR not preloaded
   * Edge-aligned mode: upcounting
   * UEV enabled: only overflow as UEV source
   * DMA requests at CCx events
   */
  _tim->CR1 = TIM_CR1_URS;

  /*
   * OC1Ref not affected by ETRF input
   * Frozen mode
   * CCR not preloaded
   * CC1 as output
   */

  /* Enable IRQ for UEV to handle sw-based repetitions */
  NVIC_SetPriority(
      tim::getIRQn(_tim, tim::UP),
      NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt, sub));
  NVIC_EnableIRQ(tim::getIRQn(_tim, tim::UP));
}

void BStepper::enable() const { _gpio->BSRR = _pins.en; }

void BStepper::disable() const { _gpio->BSRR = _pins.en << 16; }

bool BStepper::calcTimeBase(SpeedType milli_rev_per_minute, StepType t,
                            TimRegType &psc, TimRegType &arr) const {

  constexpr auto psc_width = std::numeric_limits<TimRegType>::digits;
  constexpr auto arr_width = std::numeric_limits<TimRegType>::digits;
  constexpr uint64_t max_ticks = 1ULL << (psc_width + arr_width);

  /* _steps_per_rev refers to full steps, while the setting may be HALF */
  const auto den =
      (static_cast<uint64_t>(_steps_per_rev) << t) * milli_rev_per_minute;
  const uint64_t ticks = (_sixtyk_psc_clk_hz + (den >> 1)) / den;

  if (!ticks || ticks > max_ticks)
    return false;

  /*
   *  31      16 15       0
   *  ---------------------
   * |  REM_TH  |   PSC    |
   *  ---------------------
   *
   * (PSC+1)(ARR+1) = ticks, with PSC, ARR in [0, 2^(16)-1]
   * <=> ARR = ticks/(PSC+1)-1
   * (Being PSC representable on 16 bits, the same holds for rem and rem_th)
   *
   * The candidate PSC is changed from 0 to 2^(16)-1, until one is
   * found such that ARR is representable and the remainder is below
   * the threshold. If all candidates fail, PSC wraps around and the
   * remainder threshold increases to accept worse solutions.
   *
   */

  uint_fast16_t rem;
  uint64_t arr_wide;
  uint32_t hi_rem_th_lo_psc = -1; /* Compensate starting increment */
  do {
    ++hi_rem_th_lo_psc;
    psc = hi_rem_th_lo_psc & 0xFFFFU;
    uint32_t psc_plus_one = psc + 1;
    rem = static_cast<uint_fast16_t>(ticks % psc_plus_one);
    arr_wide = ticks / psc_plus_one - 1;
  } while (rem > (hi_rem_th_lo_psc >> 16) ||
           arr_wide > std::numeric_limits<TimRegType>::max());

  arr = static_cast<TimRegType>(arr_wide);
  return true;
}

bool BStepper::rotate(StepCountType steps, SpeedType milli_rev_per_minute,
                      Direction d, StepType t) {
  if (!steps)
    return false;

  /* Get TIM configuration parameters */
  constexpr auto rcr_width = std::numeric_limits<TimRCRType>::digits;
  constexpr auto max_hw_reps = static_cast<StepCountType>(1U << rcr_width);

  TimRegType psc, arr;
  if (!calcTimeBase(milli_rev_per_minute, t, psc, arr))
    return false;

  /* Reset DMA transfer */
  LL_DMA_DisableStream(_dma, _dma_stream);

  /* Set reload period to one step time */
  LL_TIM_SetPrescaler(_tim, psc);
  LL_TIM_SetAutoReload(_tim, arr);

  /* If the number of steps fits the repetition counter,
   * force the counter to stop at the UEV */
  if (steps <= max_hw_reps) {
    /* The repetition counter is preloaded: load and force update now */
    LL_TIM_SetRepetitionCounter(_tim, steps - 1);
    LL_TIM_GenerateEvent_UPDATE(_tim);

    LL_TIM_SetOnePulseMode(_tim, LL_TIM_ONEPULSEMODE_SINGLE);
  }
  /* Otherwise, set the repetition counter to its maximum value, and
   * count the UEVs in software, allowing the counter to reload at the UEV.
   * Following the last software-counted repetition, the behavior should be
   * the same as the case when the steps fit the repetition counter */
  else {
    _sw_reps = steps >> rcr_width;
    _hw_reps = steps & (max_hw_reps - 1); /* [0, 256) */

    /* The repetition counter is preloaded: load and force update now */
    LL_TIM_SetRepetitionCounter(_tim, max_hw_reps - 1);
    LL_TIM_GenerateEvent_UPDATE(_tim);

    /* If there is only one software repetition, the next UEV
     * starts the hardware-counted repetitions. The RCR value
     * must be preloaded now */
    if (_sw_reps == 1)
      LL_TIM_SetRepetitionCounter(_tim, _hw_reps - 1);

    /* Define behavior at UEV */
    LL_TIM_SetOnePulseMode(_tim, LL_TIM_ONEPULSEMODE_REPETITIVE);
    LL_TIM_ClearFlag_UPDATE(_tim);
    LL_TIM_EnableIT_UPDATE(_tim);
  }

  /* Ensure DMA stream has been disabled, and pending requests cleared */
  while (LL_DMA_IsEnabledStream(_dma, _dma_stream));
  dma::clearFlags(_dma, _dma_stream);
  LL_TIM_DisableDMAReq_CC1(_tim);

  /* Configure DMA stream */
  LL_DMA_SetMemoryAddress(
      _dma, _dma_stream, reinterpret_cast<uintptr_t>(_tr.advance(steps, d, t)));
  LL_DMA_SetDataLength(_dma, _dma_stream, Translator::getSequenceLen(t));
  LL_DMA_EnableStream(_dma, _dma_stream);

  /* Fire DMA request just before reloading */
  LL_TIM_OC_SetCompareCH1(_tim, arr);
  LL_TIM_EnableDMAReq_CC1(_tim);

  LL_TIM_EnableCounter(_tim);
  return true;
}

void BStepper::handler() {
  if (LL_TIM_IsActiveFlag_UPDATE(_tim)) {
    LL_TIM_ClearFlag_UPDATE(_tim);

    const auto sw_rep = _sw_reps - 1;
    const auto hw_reps = _hw_reps;

    if (sw_rep == 1 && hw_reps)
      LL_TIM_SetRepetitionCounter(_tim, hw_reps - 1);
    else if (sw_rep == 1 || sw_rep == 0)
      LL_TIM_SetOnePulseMode(_tim, LL_TIM_ONEPULSEMODE_SINGLE);

    _sw_reps = sw_rep;
  }
}
