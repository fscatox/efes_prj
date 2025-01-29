/**
 * @file     exti.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     28.01.2025
 */

#include "exti.h"
#include "gpio.h"
#include "stm32f4xx_ll_system.h"

namespace exti {

void map(const GPIO_TypeDef *gpio, uint32_t pin_mask) {
  const auto pin_no = std::countr_zero(pin_mask);
  const uint32_t ll_syscfg_exti_line =
      ((0x000FUL << ((pin_no % 4) << 2)) << 16) | (pin_no >> 2);

  LL_SYSCFG_SetEXTISource(gpio::getPortNo(gpio), ll_syscfg_exti_line);
}

IRQn_Type getIRQn(uint32_t pin_mask) {
  const auto pin_no = std::countr_zero(pin_mask);
  return static_cast<IRQn_Type>(pin_no < 5    ? EXTI0_IRQn + pin_no
                                : pin_no < 10 ? EXTI9_5_IRQn
                                              : EXTI15_10_IRQn);
}

} // namespace exti