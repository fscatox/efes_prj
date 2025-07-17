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

uint32_t getAPBClockFreq(const SPI_TypeDef* spi, LL_RCC_ClocksTypeDef* clocks) {
  LL_RCC_ClocksTypeDef this_clocks;
  uint32_t fclk = 0;

  if (clocks)
    this_clocks = *clocks;
  else
    LL_RCC_GetSystemClocksFreq(&this_clocks);

  if (spi == SPI1)
    fclk = this_clocks.PCLK2_Frequency;
  else if (spi == SPI2)
    fclk = this_clocks.PCLK1_Frequency;
  else if (spi == SPI3)
    fclk = this_clocks.PCLK1_Frequency;
  else if (spi == SPI4)
    fclk = this_clocks.PCLK2_Frequency;

  return fclk;
}

}  // namespace spi