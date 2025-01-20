/**
 * @file     stm32f4xx_utils.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#include "stm32f4xx_utils.h"
#include "stm32f4xx_ll_dma.h"

const IRQn_Type DMA1_streamIRQn[] = {
    DMA1_Stream0_IRQn,
    DMA1_Stream1_IRQn,
    DMA1_Stream2_IRQn,
    DMA1_Stream3_IRQn,
    DMA1_Stream4_IRQn,
    DMA1_Stream5_IRQn,
    DMA1_Stream6_IRQn,
    DMA1_Stream7_IRQn
};

const IRQn_Type DMA2_streamIRQn[] = {
    DMA2_Stream0_IRQn,
    DMA2_Stream1_IRQn,
    DMA2_Stream2_IRQn,
    DMA2_Stream3_IRQn,
    DMA2_Stream4_IRQn,
    DMA2_Stream5_IRQn,
    DMA2_Stream6_IRQn,
    DMA2_Stream7_IRQn
};

const LL_DMA_ClearFlag_Type LL_DMA_ClearFlag_TC[] = {
    LL_DMA_ClearFlag_TC0,
    LL_DMA_ClearFlag_TC1,
    LL_DMA_ClearFlag_TC2,
    LL_DMA_ClearFlag_TC3,
    LL_DMA_ClearFlag_TC4,
    LL_DMA_ClearFlag_TC5,
    LL_DMA_ClearFlag_TC6,
    LL_DMA_ClearFlag_TC7,
};