/**
 * @file     tim.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     24.01.2025
 */

#include "tim.h"
#include "stm32f4xx_ll_rcc.h"

#define CCR_OFFSET(ch)

static constexpr void (*enable_it_cc[])(TIM_TypeDef *) {
  LL_TIM_EnableIT_CC1,
  LL_TIM_EnableIT_CC2,
  LL_TIM_EnableIT_CC3,
  LL_TIM_EnableIT_CC4,
};

static constexpr void (*disable_it_cc[])(TIM_TypeDef *) {
  LL_TIM_DisableIT_CC1,
  LL_TIM_DisableIT_CC2,
  LL_TIM_DisableIT_CC3,
  LL_TIM_DisableIT_CC4,
};

static constexpr void (*clear_flag_cc[])(TIM_TypeDef *) {
  LL_TIM_ClearFlag_CC1,
  LL_TIM_ClearFlag_CC2,
  LL_TIM_ClearFlag_CC3,
  LL_TIM_ClearFlag_CC4,
};

static constexpr uint32_t (*is_active_flag_cc[])(const TIM_TypeDef *) {
  LL_TIM_IsActiveFlag_CC1,
  LL_TIM_IsActiveFlag_CC2,
  LL_TIM_IsActiveFlag_CC3,
  LL_TIM_IsActiveFlag_CC4,
};

namespace tim {

IRQn_Type getIRQn(const TIM_TypeDef *tim, IRQAdvancedTIM irqt) {
  return getIRQn(reinterpret_cast<uintptr_t>(tim), irqt);
}

static constexpr uint32_t calcTimClk(uint32_t pclk,
                                     uint32_t apb_psc,
                                     uint32_t apb_psc_div1,
                                     uint32_t apb_psc_div2) {
  const auto &hclk = SystemCoreClock;
  const auto double_pclk = pclk << 1;

  return !LL_RCC_GetTIMPrescaler()
             ? (apb_psc == apb_psc_div1 ? hclk : double_pclk)
         : apb_psc == apb_psc_div1 || apb_psc == apb_psc_div2
             ? hclk
             : double_pclk << 1;
}

uint32_t getPscClock(uintptr_t base_addr) {
  const auto &hclk = SystemCoreClock;

  switch (base_addr) {
    case TIM1_BASE:
    case TIM9_BASE:
    case TIM10_BASE:
    case TIM11_BASE: {
      const auto apb2_psc = LL_RCC_GetAPB2Prescaler();
      const auto pclk2 = __LL_RCC_CALC_PCLK2_FREQ(hclk, apb2_psc);
      return calcTimClk(pclk2, apb2_psc, LL_RCC_APB2_DIV_1, LL_RCC_APB2_DIV_2);
    }

    case TIM2_BASE:
    case TIM3_BASE:
    case TIM4_BASE:
    case TIM5_BASE: {
      const auto apb1_psc = LL_RCC_GetAPB1Prescaler();
      const auto pclk1 = __LL_RCC_CALC_PCLK1_FREQ(hclk, apb1_psc);
      return calcTimClk(pclk1, apb1_psc, LL_RCC_APB1_DIV_1, LL_RCC_APB1_DIV_2);
    }

    default:
      return 0;
  }
}

uint32_t getPscClock(const TIM_TypeDef *tim) {
  return getPscClock(reinterpret_cast<uintptr_t>(tim));
};

void enableClock(const TIM_TypeDef *tim) {
  enableClock(reinterpret_cast<uintptr_t>(tim));
}

void enableItCC(TIM_TypeDef *tim, size_t ch) {
  enable_it_cc[ch](tim);
}

void disableItCC(TIM_TypeDef *tim, size_t ch) {
  disable_it_cc[ch](tim);
}

void clearFlagCC(TIM_TypeDef *tim, size_t ch) {
  clear_flag_cc[ch](tim);
}

bool isActiveFlagCC(const TIM_TypeDef *tim, size_t ch) {
  return is_active_flag_cc[ch](tim);
}

}