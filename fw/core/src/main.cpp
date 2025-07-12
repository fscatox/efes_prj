/**
 * @file     main.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 */

#include "main.h"
#include "debug.h"
#include "gpio.h"

#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"

#include "MotionPattern.hpp"
#include "SSegDisplay.hpp"
#include "UartTx.hpp"
#include "Keyboard.hpp"

using namespace std::chrono_literals;

/* Lazy construction of character devices */
using StLinkUartTxType = UartTx<>;
static StLinkUartTxType &St_Link_Uart_Tx();

using SSegDisplayType = SSegDisplay<HwAlarmType, 80, false>;
static SSegDisplayType &SSeg_Display();

/* Platform configuration */
static void systemClockConfig();

[[noreturn]] int main() {

  /* Configure system clock tree */
  systemClockConfig();

  /* Initialize GPIO ports */
  gpio::init();

  /* Initialize logging facility */
  St_Link_Uart_Tx().setPin(USART_TX_GPIO_Port, USART_TX_Pin,
                           USART_TX_Alternate);
  St_Link_Uart_Tx().setFrame(LL_USART_PARITY_NONE, LL_USART_STOPBITS_1);
  St_Link_Uart_Tx().setBaudRate(115200, LL_USART_OVERSAMPLING_16);
  St_Link_Uart_Tx().setDMATransfer(LL_DMA_STREAM_6, LL_DMA_CHANNEL_4);
  File_Manager().stdStreamAttach(STDERR_FILENO, "st_link_uart_tx");
  File_Manager().stdStreamAttach(STDOUT_FILENO, "st_link_uart_tx");
  PRINTD("Logging facility running ...");

  /* Hardware timer: ticks @ 64 kHz, (2 channels, 16 bit) */
  Hw_Alarm().init(15625ns);

  /* Initialize push button */
  Push_Button().init();
  Push_Button().enable();

  /* Initialize stepper motor */
  Stepper().setPins({.en = EN_Pin,
                     .ph = {.a = {.pos = AP_Pin, .neg = AN_Pin},
                            .b = {.pos = BP_Pin, .neg = BN_Pin}}});
  Stepper().setDMATransfer(LL_DMA_STREAM_1, LL_DMA_CHANNEL_6);
  Stepper().setResolution(200);
  Stepper().init();

  /* Initialize motion pattern */
  constexpr auto nmax_motion_segments = 128;
  MotionPattern<nmax_motion_segments> mp(flash::Sector::S7);
  PRINTD("MotionPatter cache: %u/%u", mp.size(), mp.max_size());

  /*
   * Initialize device drivers
   */

  /* 7-Segment display peripheral over USART1 */
  constexpr auto display_len = 6;
  SSeg_Display().setPin(SSEG_URX_GPIO_Port, SSEG_URX_Pin, SSEG_URX_Alternate);
  SSeg_Display().setFrame(LL_USART_PARITY_EVEN, LL_USART_STOPBITS_1);
  SSeg_Display().setBaudRate(115200, LL_USART_OVERSAMPLING_16);
  SSeg_Display().setDMATransfer(LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);
  SSeg_Display().setDisplay(display_len, 250ms);

  auto display_out = fopen("sseg_display", "w");
  if (!display_out)
    exit(-1);

  while (true) {
    if (Push_Button().shortPress()) {
      PRINTD("mp.emplaceBack() -> 0x%08p", mp.emplaceBack(0xAAAAAAAA,0xAAAA,BStepper::CCW));
      PRINTD("MotionPatter cache: %u/%u", mp.size(), mp.max_size());
    } else if (Push_Button().longPress()) {
      mp.clear();
      PRINTD("MotionPatter cache: %u/%u", mp.size(), mp.max_size());
    }
  }
}

static void systemClockConfig() {
  const uint32_t HCLK_FREQUENCY_HZ = 64000000;

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
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8,
                              HCLK_FREQUENCY_HZ / 1000000, LL_RCC_PLLP_DIV_2);

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

static StLinkUartTxType &St_Link_Uart_Tx() {
  static StLinkUartTxType obj(USART2, DMA1);
  return obj;
}

static SSegDisplayType &SSeg_Display() {
  static SSegDisplayType obj(USART1, DMA2, Hw_Alarm());
  return obj;
}

FileManagerType &File_Manager() {
  static FileManagerType fm{Node{"st_link_uart_tx", St_Link_Uart_Tx()},
                            Node{"sseg_display", SSeg_Display()}};
  return fm;
}

HwAlarmType &Hw_Alarm() {
  static HwAlarmType obj;
  return obj;
}

PushButtonType &Push_Button() {
  static PushButtonType obj{B1_GPIO_Port, B1_Pin, Hw_Alarm()};
  return obj;
}

BStepper &Stepper() {
  static BStepper obj{AB_GPIO_Port, TIM1, DMA2};
  return obj;
}
