/**
 * @file     exti.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     28.01.2025
 */

#ifndef EXTI_H
#define EXTI_H

#include "stm32f4xx_ll_exti.h"

namespace exti {

void map(const GPIO_TypeDef *gpio, uint32_t pin_mask);

IRQn_Type getIRQn(uint32_t pin_mask);

} // namespace exti

#endif // EXTI_H