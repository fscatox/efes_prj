/**
 * @file     TimeBase.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.01.2025
 */

#include "TimeBase.h"
#include "stm32f4xx.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_cortex.h"

volatile TickType_t ms_ticks = 0;

void timeBaseInit(void) {
  LL_InitTick(SystemCoreClock, 1000);
  LL_SYSTICK_EnableIT();
}

void SysTick_Handler(void) {
  ms_ticks++;
}
