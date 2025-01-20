/**
 * @file     UartTx.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#ifndef UARTTX_HPP
#define UARTTX_HPP

#include "IFile.h"

#include "stm32f4xx_ll_dma.h"
#include "stm32f4xx_ll_gpio.h"
#include "stm32f4xx_ll_usart.h"

#include <array>

template <size_t BUF_SIZE = 512>
class UartTx final : public IFile {
public:
  UartTx(USART_TypeDef *usart, DMA_TypeDef *dma);

  void setPin(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af);
  void setFrame(uint32_t data_width, uint32_t parity, uint32_t stop);
  void setBaudRate(uint32_t baud_rate, uint32_t over_sampling);
  void setDMATransfer(uint32_t stream, uint32_t ch,
                      uint32_t stream_priority = LL_DMA_PRIORITY_LOW);

  int open(const OFile &ofile) override;
  int close(const OFile &ofile) override;

  off_t llseek(OFile &ofile, off_t offset, int whence) override;
  ssize_t write(const OFile &ofile, const void *buf, size_t count,
                off_t &pos) override;

private:
  USART_TypeDef *_usart;
  DMA_TypeDef *_dma;
  GPIO_TypeDef *_gpio;

  LL_USART_InitTypeDef _usart_init;
  uint32_t _dma_stream;
  LL_DMA_InitTypeDef _dma_init;
  LL_GPIO_InitTypeDef _gpio_init;

  using Buffer = std::array<uint8_t, BUF_SIZE>;
  Buffer _buf;
};

#include "UartTx.tpp"

#endif // UARTTX_HPP
