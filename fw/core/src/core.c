/**
 * @file     core.c
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.01.2025
 */

#include "core.h"

#include "stm32f4xx_ll_cortex.h"
#include "stm32f4xx_ll_utils.h"

static volatile TickType_t ms_ticks = 0;

void sysTickInit(void) {
  LL_InitTick(SystemCoreClock, 1000);
  LL_SYSTICK_EnableIT();
}

inline TickType_t sysTickTimeMs(void) {
  return ms_ticks;
}

void SysTick_Handler(void) {
  ms_ticks++;
}
