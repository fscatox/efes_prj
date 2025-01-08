/**
 * @file     main.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 */

#include "main.h"
#include <stm32f4xx_ll_bus.h>
#include <stm32f4xx_ll_pwr.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_system.h>
#include <stm32f4xx_ll_utils.h>

#include "TimeBase.h"
#include "gpio.h"

#include "PushButton.h"

void toggleLed() { LL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); }
void clearLed() { LL_GPIO_ResetOutputPin(LD2_GPIO_Port, LD2_Pin); }
PushButton ubutton(B1_GPIO_Port, B1_Pin, toggleLed, clearLed);

extern "C" void EXTI15_10_IRQHandler() {
  ubutton.extiHandler();
}

__NO_RETURN int main() {

  /* Configure system clock tree */
  systemClockConfig();

  /* Start systick time base */
  timeBaseInit();

  /* Initialize GPIO ports */
  gpioInit();

  /* Initialize push button */
  ubutton.init();

  /* Configure peripheral interrupts */
  NVIC_EnableIRQ(ubutton.irqn());
  ubutton.enableIt();

  while (true) {
    ubutton.run();
  }
}

void systemClockConfig(void) {

  /* On reset, the 16 MHz internal RC oscillator is selected.
   * Enable clock to power interface and system configuration controller */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);

  /* The latency defaults to zero. For the target HCLK and a supply voltage
   * range (2.7 V to 3.6 V), the latency is increased to 2 */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_2);
  while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_2);

  /* The voltage regulator is set to scale mode 2, which kicks in once
   * the PLL is enabled */
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

  /* Configure the PLL to generate the target HCLK from HSI */
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI,
    LL_RCC_PLLM_DIV_8, HCLK_FREQUENCY_HZ/1000000, LL_RCC_PLLP_DIV_2);

  /* Configure peripherals clock tree
   * HPRE   (AHB): 0x0000, /1
   * PPRE1 (APB1): 0x100, /2
   * PPRE2 (APB2): 0x100, /2
   */
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);

  /* Enable PLL */
  LL_RCC_PLL_Enable();
  while (!LL_RCC_PLL_IsReady());

  /* Switch to PLL clock */
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

  /* Update CMSIS SystemCoreClock variable */
  LL_SetSystemCoreClock(HCLK_FREQUENCY_HZ);

}
