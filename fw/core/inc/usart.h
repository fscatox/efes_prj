/**
 * @file     usart.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#ifndef USART_H
#define USART_H

#include "stm32f4xx_ll_usart.h"

namespace usart {

void enableClock(USART_TypeDef *usart);

}

#endif //USART_H
