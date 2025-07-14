/**
 * @file     main.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     05.01.2025
 */

#include "main.h"

#include "Keyboard.hpp"
#include "MotionPattern.hpp"
#include "SSegDisplay.hpp"
#include "UartTx.hpp"
#include "ctre.hpp"
#include "debug.h"
#include "gpio.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"

using namespace std::chrono_literals;

/* Lazy construction of character devices */
using StLinkUartTxType = UartTx<>;
static StLinkUartTxType &St_Link_Uart_Tx();

using SSegDisplayType = SSegDisplay<HwAlarmType, 80, false>;
static SSegDisplayType &SSeg_Display();

/* Platform configuration */
static void systemClockConfig();

/* Utility */
template <typename T, typename U,
          std::enable_if_t<std::is_signed_v<T>, bool> = true>
static constexpr std::pair<T, T> makeFixed(T x, U factor) {
  return std::make_pair(x / factor, std::abs(x % factor));
}
template <typename T, typename U,
          std::enable_if_t<std::is_unsigned_v<T>, bool> = true>
static constexpr std::pair<T, T> makeFixed(T x, U factor) {
  return std::make_pair(x / factor, x % factor);
}

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
  constexpr auto stepper_resolution = 200;
  Stepper().setPins({.en = EN_Pin,
                     .ph = {.a = {.pos = AP_Pin, .neg = AN_Pin},
                            .b = {.pos = BP_Pin, .neg = BN_Pin}}});
  Stepper().setDMATransfer(LL_DMA_STREAM_1, LL_DMA_CHANNEL_6);
  Stepper().setResolution(stepper_resolution);
  Stepper().init();

  /* Initialize motion pattern */
  constexpr auto nmax_motion_segments = 128;
  MotionPattern<nmax_motion_segments> mp(flash::Sector::S7);
  PRINTD("MotionPatter cache: %u/%u", mp.size(), mp.max_size());

  /* Initialize keyboard over SPI3 */
  constexpr auto nmax_chars = 32;
  Keyboard<HwAlarmType, nmax_chars> kb(SPI3, Hw_Alarm());
  kb.setPins({MISO_GPIO_Port, MISO_Pin, MISO_GPIO_Alternate},
             {MOSI_GPIO_Port, MOSI_Pin, MOSI_GPIO_Alternate},
             {SCLK_GPIO_Port, SCLK_Pin, SCLK_GPIO_Alternate},
             {PS2_SS_N_GPIO_Port, PS2_SS_N_Pin, 0});
  kb.setFormat(LL_SPI_POLARITY_LOW, LL_SPI_PHASE_1EDGE);
  kb.setBaudRate(LL_SPI_BAUDRATEPRESCALER_DIV4);

  /* 7-Segment display peripheral over USART1 */
  constexpr auto display_len = 6;
  SSeg_Display().setPin(SSEG_URX_GPIO_Port, SSEG_URX_Pin, SSEG_URX_Alternate);
  SSeg_Display().setFrame(LL_USART_PARITY_EVEN, LL_USART_STOPBITS_1);
  SSeg_Display().setBaudRate(115200, LL_USART_OVERSAMPLING_16);
  SSeg_Display().setDMATransfer(LL_DMA_STREAM_7, LL_DMA_CHANNEL_4);
  SSeg_Display().setDisplay(display_len, 250ms);

  auto display_out = fopen("sseg_display", "w");
  if (!display_out) exit(-1);

  while (true) {
    if (Push_Button().longPress(true)) {
      mp.clear();
      fprintf(display_out, "Clear\n");
      PRINTD("MotionPatter cache cleared: %u/%u", mp.size(), mp.max_size());
      Push_Button().enable();
    } else if (Push_Button().shortPress()) {
      /* Triggered movement pattern execution */
      if (!mp.empty()) {
        fprintf(display_out, "Play\n");

        Stepper().enable();
        PRINTD("Starting movement pattern execution");

        auto ms_it = mp.begin();
        do {
          Stepper().rotate(ms_it->steps, ms_it->milli_rev_per_minute,
                           ms_it->direction);

          if (++ms_it == mp.end()) ms_it = mp.begin();
        } while (!Push_Button().shortPress());

        Stepper().disable();
        PRINTD("Stopped movement pattern execution");

      } else {
        fprintf(display_out, "Err-1\n");
        PRINTD("Movement pattern execution aborted");
      }
    }

    if (kb.available()) {
      /* Data point entry triggered: notify to SSegDisplay */
      fprintf(display_out, "Input\n");

      /* Acquire full line blocking */
      auto str = kb.getLine();
      PRINTD("kb.getLine() -> '%.*s'", str.size(), str.begin());

      /* Check for available chunk in NVS */
      if (const auto mp_idx = mp.size(); mp_idx < mp.max_size()) {
        /* Validate line against regex pattern */
        if (const auto mr =
                ctre::match<R"((?:\+|-)?([0-9]{1,3})(?:\.([0-9]))?\n)">(
                    str.cbegin(), str.cend())) {
          /* Extract angular displacement */
          auto angle_x10 = mr.get<1>().to_number() * 10 +
                           (mr.get<2>() ? mr.get<2>().to_number() : 0);

          /* Construct data point */
          const MotionPattern<nmax_motion_segments>::MotionSegment dp{
              .milli_rev_per_minute = 200'000,
              .steps = static_cast<BStepper::StepCountType>(
                  (angle_x10 * (stepper_resolution / 10) + 180) / 360),
              .direction = str.front() != '-' ? BStepper::CCW : BStepper::CW};

          /* Log to SSegDisplay */
          angle_x10 = (dp.direction == BStepper::CCW ? 1 : -1) *
                      ((dp.steps * 3600) / stepper_resolution);
          const auto rpm = makeFixed(dp.milli_rev_per_minute, 1000);
          const auto sgn_angle = makeFixed(angle_x10, 10);

          fprintf(display_out, "[%u] %lu.%lu %d.%d\n", mp_idx, rpm.first,
                  rpm.second, sgn_angle.first, sgn_angle.second);
          PRINTD("Data point: [%u] %lu.%lu %d.%d", mp_idx, rpm.first,
                 rpm.second, sgn_angle.first, sgn_angle.second);

          /* Commit to NVS */
          if (!mp.emplaceBack(dp)) PRINTE("Failed to commit data point to NVS");

        } else {
          fprintf(display_out, "Err-2\n");
          PRINTD("Line validation failed");
        }

      } else {
        fprintf(display_out, "Err-3\n");
        PRINTD("MotionPattern cache is full: %u/%u", mp_idx, mp.max_size());
      }

      /* Discard processed line */
      kb.clear();
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
