/**
 * @file     usart.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#include "usart.h"
#include "stm32f4xx_ll_bus.h"

namespace usart {

void enableClock(USART_TypeDef *usart) {
  if (usart == USART1)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);
  else if (usart == USART2)
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART2);
  else if (usart == USART6)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART6);
}

}
