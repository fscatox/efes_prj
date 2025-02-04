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

static constexpr void (*clear_flag_ht[])(DMA_TypeDef *) {
  LL_DMA_ClearFlag_HT0,
  LL_DMA_ClearFlag_HT1,
  LL_DMA_ClearFlag_HT2,
  LL_DMA_ClearFlag_HT3,
  LL_DMA_ClearFlag_HT4,
  LL_DMA_ClearFlag_HT5,
  LL_DMA_ClearFlag_HT6,
  LL_DMA_ClearFlag_HT7
};

static constexpr void (*clear_flag_te[])(DMA_TypeDef *) {
  LL_DMA_ClearFlag_TE0,
  LL_DMA_ClearFlag_TE1,
  LL_DMA_ClearFlag_TE2,
  LL_DMA_ClearFlag_TE3,
  LL_DMA_ClearFlag_TE4,
  LL_DMA_ClearFlag_TE5,
  LL_DMA_ClearFlag_TE6,
  LL_DMA_ClearFlag_TE7
};

static constexpr void (*clear_flag_dme[])(DMA_TypeDef *) {
  LL_DMA_ClearFlag_DME0,
  LL_DMA_ClearFlag_DME1,
  LL_DMA_ClearFlag_DME2,
  LL_DMA_ClearFlag_DME3,
  LL_DMA_ClearFlag_DME4,
  LL_DMA_ClearFlag_DME5,
  LL_DMA_ClearFlag_DME6,
  LL_DMA_ClearFlag_DME7
};

static constexpr void (*clear_flag_fe[])(DMA_TypeDef *) {
  LL_DMA_ClearFlag_FE0,
  LL_DMA_ClearFlag_FE1,
  LL_DMA_ClearFlag_FE2,
  LL_DMA_ClearFlag_FE3,
  LL_DMA_ClearFlag_FE4,
  LL_DMA_ClearFlag_FE5,
  LL_DMA_ClearFlag_FE6,
  LL_DMA_ClearFlag_FE7
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

void clearFlagHT(DMA_TypeDef *dma, uint32_t stream) {
  clear_flag_ht[stream](dma);
}

void clearFlagTE(DMA_TypeDef *dma, uint32_t stream) {
  clear_flag_te[stream](dma);
}

void clearFlagDME(DMA_TypeDef *dma, uint32_t stream) {
  clear_flag_dme[stream](dma);
}

void clearFlagFE(DMA_TypeDef *dma, uint32_t stream) {
  clear_flag_fe[stream](dma);
}

void clearFlags(DMA_TypeDef *dma, uint32_t stream) {
  clearFlagTC(dma, stream);
  clearFlagHT(dma, stream);
  clearFlagTE(dma, stream);
  clearFlagDME(dma, stream);
  clearFlagFE(dma, stream);
}

}
