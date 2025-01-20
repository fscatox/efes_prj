/**
 * @file     stm32f4xx_utils.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#ifndef STM32F4XX_UTILS_H
#define STM32F4XX_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stm32f4xx_ll_bus.h>

__STATIC_INLINE uint32_t GPIO_getPortNo(GPIO_TypeDef *gpio) {
  uint32_t gpio_base = (uintptr_t)gpio;
  return (gpio_base - AHB1PERIPH_BASE) >> 10;
}

__STATIC_INLINE void GPIO_enableClock(GPIO_TypeDef *gpio) {
  auto gpio_no = GPIO_getPortNo(gpio);
  LL_AHB1_GRP1_EnableClock(0x1UL << gpio_no);
}

__STATIC_INLINE void USART_enableClock(USART_TypeDef *usart) {
  if (usart == USART1)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  else if (usart == USART2)
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  else if (usart == USART6)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
}

extern const IRQn_Type DMA1_streamIRQn[];
extern const IRQn_Type DMA2_streamIRQn[];

typedef void (*LL_DMA_ClearFlag_Type)(DMA_TypeDef *);
extern const LL_DMA_ClearFlag_Type LL_DMA_ClearFlag_TC[];

__STATIC_INLINE void DMA_enableClock(DMA_TypeDef *dma) {
  if (dma == DMA1)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  else if (dma == DMA2)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
}

__STATIC_INLINE IRQn_Type DMA_getStreamIRQn(DMA_TypeDef *dma, uint32_t stream) {
  if (dma == DMA1)
    return DMA1_streamIRQn[stream];
  if (dma == DMA2)
    return DMA2_streamIRQn[stream];
}

#ifdef __cplusplus
}
#endif

#endif //STM32F4XX_UTILS_H
