/**
 * @file     dma.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#include <cstdlib>

#include "dma.h"
#include "stm32f4xx_ll_bus.h"

static constexpr void (*clear_flag_tc[])(DMA_TypeDef *) {
  LL_DMA_ClearFlag_TC0,
  LL_DMA_ClearFlag_TC1,
  LL_DMA_ClearFlag_TC2,
  LL_DMA_ClearFlag_TC3,
  LL_DMA_ClearFlag_TC4,
  LL_DMA_ClearFlag_TC5,
  LL_DMA_ClearFlag_TC6,
  LL_DMA_ClearFlag_TC7
};

namespace dma {

void enableClock(DMA_TypeDef *dma) {
  if (dma == DMA1)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  else if (dma == DMA2)
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA2);
}

void clearFlagTC(DMA_TypeDef *dma, uint32_t stream) {
  clear_flag_tc[stream](dma);
}

}
