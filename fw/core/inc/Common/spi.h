/**
 * @file     spi.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     12.07.2025
 */

#ifndef SPI_H
#define SPI_H

#include "stm32f4xx_ll_spi.h"

namespace spi {

void enableClock(const SPI_TypeDef *spi);

}

#endif //SPI_H
