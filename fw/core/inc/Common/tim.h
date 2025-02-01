/**
 * @file     tim.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     24.01.2025
 */

#ifndef TIM_H
#define TIM_H

#include "stm32f4xx_ll_tim.h"
#include "stm32f4xx_ll_bus.h"

#include <cstddef>
#include <type_traits>

namespace tim {

enum IRQAdvancedTIM {
  NONE = NonMaskableInt_IRQn - 1,
  BRK = static_cast<int>(TIM1_BRK_TIM9_IRQn),
  UP = static_cast<int>(TIM1_UP_TIM10_IRQn),
  TRG_COM = static_cast<int>(TIM1_TRG_COM_TIM11_IRQn),
  CC = static_cast<int>(TIM1_CC_IRQn)
};

constexpr IRQn_Type getIRQn(uintptr_t base_addr, IRQAdvancedTIM irqt = NONE) {
  switch (base_addr) {
  case TIM1_BASE:
    return static_cast<IRQn_Type>(irqt);
  case TIM2_BASE:
    return TIM2_IRQn;
  case TIM3_BASE:
    return TIM3_IRQn;
  case TIM4_BASE:
    return TIM4_IRQn;
  case TIM5_BASE:
    return TIM5_IRQn;
  case TIM9_BASE:
    return TIM1_BRK_TIM9_IRQn;
  case TIM10_BASE:
    return TIM1_UP_TIM10_IRQn;
  case TIM11_BASE:
    return TIM1_TRG_COM_TIM11_IRQn;
  default:
    return static_cast<IRQn_Type>(NonMaskableInt_IRQn - 1);
  }
}

constexpr uint32_t getNChannels(uintptr_t base_addr) {
  switch (base_addr) {
  case TIM1_BASE:
  case TIM2_BASE:
  case TIM3_BASE:
  case TIM4_BASE:
  case TIM5_BASE:
    return 4;

  case TIM9_BASE:
    return 2;

  case TIM10_BASE:
  case TIM11_BASE:
    return 1;

  default:
    return 0;
  }
}

constexpr uint32_t getWidth(uintptr_t base_addr) {
  switch (base_addr) {
  case TIM2_BASE:
  case TIM5_BASE:
    return 32;

  case TIM1_BASE:
  case TIM3_BASE:
  case TIM4_BASE:
  case TIM9_BASE:
  case TIM10_BASE:
  case TIM11_BASE:
    return 16;

  default:
    return 0;
  }
}

constexpr bool is32Bit(uintptr_t base_addr) {
  return getWidth(base_addr) == 32;
}

uint32_t getPscClock(uintptr_t base_addr);

constexpr void enableClock(uintptr_t base_addr) {
  switch (base_addr) {
  case TIM1_BASE:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM1);
    return;
  case TIM9_BASE:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM9);
    return;
  case TIM10_BASE:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM10);
    return;
  case TIM11_BASE:
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_TIM11);
    return;

  case TIM2_BASE:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);
    return;
  case TIM3_BASE:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);
    return;
  case TIM4_BASE:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);
    return;
  case TIM5_BASE:
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM5);
    return;
  }
}

constexpr uintptr_t ccrAddr(uintptr_t base_addr, size_t ch) {
  switch (ch) {
  case 0:
    return base_addr + offsetof(TIM_TypeDef, CCR1);
  case 1:
    return base_addr + offsetof(TIM_TypeDef, CCR2);
  case 2:
    return base_addr + offsetof(TIM_TypeDef, CCR3);
  case 3:
    return base_addr + offsetof(TIM_TypeDef, CCR4);
  default:
    return 0;
  }
}

template <uintptr_t TimBase>
auto ccr(size_t ch)
    -> volatile std::conditional_t<is32Bit(TimBase), uint32_t, uint16_t> * {
  return reinterpret_cast<
      volatile std::conditional_t<is32Bit(TimBase), uint32_t, uint16_t> *>(
      ccrAddr(TimBase, ch));
}

void enableItCC(TIM_TypeDef *tim, size_t ch);

void disableItCC(TIM_TypeDef *tim, size_t ch);

void clearFlagCC(TIM_TypeDef *tim, size_t ch);

bool isActiveFlagCC(const TIM_TypeDef *tim, size_t ch);

} // namespace tim

#endif // TIM_H
