/**
 * @file     gpio.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.01.2025
 */

#ifndef GPIO_H
#define GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Pinout
 */

/* MCU push button and LED */
#define B1_Pin LL_GPIO_PIN_13
#define B1_GPIO_Port GPIOC

#define LD2_Pin LL_GPIO_PIN_5
#define LD2_GPIO_Port GPIOA

/* FPGA */

/* asynchronous reset */
#define ASYNC_RST_N_Pin LL_GPIO_PIN_0
#define ASYNC_RST_N_GPIO_Port GPIOC

/* SPI peripherals: ADC and PS/2 controller */
#define SCLK_Pin LL_GPIO_PIN_10
#define SCLK_GPIO_Port GPIOC

#define MISO_Pin LL_GPIO_PIN_11
#define MISO_GPIO_Port GPIOC

#define MOSI_Pin LL_GPIO_PIN_12
#define MOSI_GPIO_Port GPIOC

#define ADC_SS_N_Pin LL_GPIO_PIN_8
#define ADC_SS_N_GPIO_Port GPIOC

#define PS2_SS_N_Pin LL_GPIO_PIN_9
#define PS2_SS_N_GPIO_Port GPIOC

/* 7-Segment displays peripheral */
#define SSEG_URX_Pin LL_GPIO_PIN_6
#define SSEG_URX_GPIO_Port GPIOB

/* Motor board */
#define EN_Pin LL_GPIO_PIN_1
#define EN_GPIO_Port GPIOC

#define AP_Pin LL_GPIO_PIN_8
#define AP_GPIO_Port GPIOA

#define AN_Pin LL_GPIO_PIN_7
#define AN_GPIO_Port GPIOA

#define BP_Pin LL_GPIO_PIN_10
#define BP_GPIO_Port GPIOA

#define BN_Pin LL_GPIO_PIN_1
#define BN_GPIO_Port GPIOB

/* ST-Link */
#define USART_TX_Pin LL_GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA

#define USART_RX_Pin LL_GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA

#define SWDIO_Pin LL_GPIO_PIN_13
#define SWDIO_GPIO_Port GPIOA

#define SWCLK_Pin LL_GPIO_PIN_14
#define SWCLK_GPIO_Port GPIOA

/* Initialize used and unused pins */
void gpioInit(void);

#ifdef __cplusplus
}
#endif

#endif // GPIO_H
