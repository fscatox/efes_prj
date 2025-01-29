/**
 * @file     dma.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#ifndef DMA_H
#define DMA_H

#include "stm32f4xx_ll_dma.h"

namespace dma {

void enableClock(DMA_TypeDef *dma);
void clearFlagTC(DMA_TypeDef *dma, uint32_t stream);

}

#endif //DMA_H
