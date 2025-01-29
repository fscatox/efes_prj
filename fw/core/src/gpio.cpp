/**
 * @file     gpio.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     23.01.2025
 */

#include "gpio.h"
#include "stm32f4xx_ll_bus.h"
#include <cstddef>

namespace gpio {

int getPortNo(const GPIO_TypeDef *gpio) {
  const auto gpio_base = reinterpret_cast<uintptr_t>(gpio);
  return (gpio_base - AHB1PERIPH_BASE) >> 10;
}

void enableClock(const GPIO_TypeDef *gpio) {
  const auto gpio_no = getPortNo(gpio);
  LL_AHB1_GRP1_EnableClock(0x1UL << gpio_no);
}

/**
 * Unused pins are configured as analog inputs with no pull-up/down.
 * Unused ports are clock gated.
 */
static void initUnused() {

  LL_GPIO_InitTypeDef init_struct = {.Mode = LL_GPIO_MODE_ANALOG};

  GPIO_TypeDef *ports[] = {GPIOA, GPIOB, GPIOC, GPIOD, GPIOH};

  uint32_t msks[] = {
    /* GPIOA */
    LL_GPIO_PIN_ALL ^ (LD2_Pin | AP_Pin | AN_Pin | BP_Pin | USART_TX_Pin |
                       USART_RX_Pin | SWDIO_Pin | SWCLK_Pin),

    /* GPIOB */
    LL_GPIO_PIN_ALL ^ (SSEG_URX_Pin | BN_Pin),

    /* GPIOC */
    LL_GPIO_PIN_ALL ^ (B1_Pin | ASYNC_RST_N_Pin | SCLK_Pin | MISO_Pin |
                       MOSI_Pin | ADC_SS_N_Pin | PS2_SS_N_Pin | EN_Pin),

    /* GPIOD */
    LL_GPIO_PIN_2,

    /* GPIOH */
    LL_GPIO_PIN_0 | LL_GPIO_PIN_1};

  for (size_t i = 0; i < sizeof(ports) / sizeof(ports[0]); i++) {
    init_struct.Pin = msks[i];
    LL_GPIO_Init(ports[i], &init_struct);
  }

  LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_DisableClock(LL_AHB1_GRP1_PERIPH_GPIOH);
}

void init() {

  /* Enable clock to GPIO ports */
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOD);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOH);

  /* User LED on */
  LL_GPIO_SetOutputPin(LD2_GPIO_Port, LD2_Pin);
  LL_GPIO_SetPinMode(LD2_GPIO_Port, LD2_Pin, LL_GPIO_MODE_OUTPUT);

  /* FPGA subsystem kept under reset */
  LL_GPIO_SetPinMode(ASYNC_RST_N_GPIO_Port, ASYNC_RST_N_Pin,
                     LL_GPIO_MODE_OUTPUT);

  /* SPI ADC and PS/2 controller not selected */
  LL_GPIO_SetOutputPin(ADC_SS_N_GPIO_Port, ADC_SS_N_Pin);
  LL_GPIO_SetOutputPin(PS2_SS_N_GPIO_Port, PS2_SS_N_Pin);

  LL_GPIO_SetPinMode(ADC_SS_N_GPIO_Port, ADC_SS_N_Pin, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(PS2_SS_N_GPIO_Port, PS2_SS_N_Pin, LL_GPIO_MODE_OUTPUT);

  /* Motor board disabled */
  LL_GPIO_SetPinMode(EN_GPIO_Port, EN_Pin, LL_GPIO_MODE_OUTPUT);

  initUnused();
}

}
