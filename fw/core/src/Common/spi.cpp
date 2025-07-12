/**
 * @file     spi.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.07.2025
 */

#include "spi.h"

#include <stm32f4xx_ll_bus.h>

namespace spi {

void enableClock(const SPI_TypeDef* spi) {
  if (spi == SPI1)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  else if (spi == SPI2)
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI2);
  else if (spi == SPI3)
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_SPI3);
  else if (spi == SPI4)
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI4);
}

}  // namespace spi