/**
 * @file     UartTx.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#ifndef UARTTX_TPP
#define UARTTX_TPP

#include <algorithm>
#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "gpio.h"
#include "dma.h"
#include "usart.h"

template <size_t BUF_SIZE>
UartTx<BUF_SIZE>::UartTx(USART_TypeDef *usart, DMA_TypeDef *dma)
    : _usart(usart), _dma(dma), _gpio(nullptr),
      _usart_init{.TransferDirection = LL_USART_DIRECTION_TX,
                  .HardwareFlowControl = LL_USART_HWCONTROL_NONE},
      _dma_init{
          .PeriphOrM2MSrcAddress = reinterpret_cast<uintptr_t>(&_usart->DR),
          .Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
          .Mode = LL_DMA_MODE_NORMAL,
          .PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT,
          .MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT,
          .PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE,
          .MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE,
          .FIFOMode = LL_DMA_FIFOMODE_DISABLE,
      },
      _gpio_init{.Mode = LL_GPIO_MODE_ALTERNATE,
                 .Speed = LL_GPIO_SPEED_FREQ_LOW,
                 .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
                 .Pull = LL_GPIO_PULL_NO} {}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setPin(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af) {
  _gpio = gpio;
  _gpio_init.Pin = pin;
  _gpio_init.Alternate = af;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setFrame(uint32_t parity, uint32_t stop) {
  _usart_init.DataWidth = (parity == LL_USART_PARITY_NONE) ? LL_USART_DATAWIDTH_8B
                                                           : LL_USART_DATAWIDTH_9B;
  _usart_init.Parity = parity;
  _usart_init.StopBits = stop;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setBaudRate(uint32_t baud_rate, uint32_t over_sampling) {
  _usart_init.BaudRate = baud_rate;
  _usart_init.OverSampling = over_sampling;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setDMATransfer(uint32_t stream, uint32_t ch,
                            uint32_t stream_priority) {
  _dma_stream = stream;
  _dma_init.Priority = stream_priority;
  _dma_init.Channel = ch;
  _dma_init.MemoryOrM2MDstAddress = reinterpret_cast<uintptr_t>(_buf.data());
}

template <size_t BUF_SIZE>
int UartTx<BUF_SIZE>::open(OFile &ofile) {
  if (ofile.mode != FWRITE)
    return -EINVAL;

  /* Initialize GPIO peripheral */
  gpio::enableClock(_gpio);
  LL_GPIO_Init(_gpio, &_gpio_init);

  /* Initialize DMA peripheral */
  dma::enableClock(_dma);
  LL_DMA_Init(_dma, _dma_stream, &_dma_init);

  /* Initialize USART peripheral */
  usart::enableClock(_usart);
  LL_USART_Init(_usart, &_usart_init);

  /* Enable USART in DMA mode */
  LL_USART_EnableDMAReq_TX(_usart);
  LL_USART_Enable(_usart);

  return 0;
}

template <size_t BUF_SIZE>
int UartTx<BUF_SIZE>::close([[maybe_unused]] OFile &ofile) {
  LL_GPIO_InitTypeDef gpio_init = _gpio_init;
  gpio_init.Pin = _gpio_init.Mode = LL_GPIO_MODE_ANALOG;

  /* Wait for ongoing transfer to complete */
  while (!LL_USART_IsActiveFlag_TC(_usart));

  /* De-init peripherals */
  LL_USART_DeInit(_usart);
  LL_DMA_DeInit(_dma, _dma_stream);
  LL_GPIO_Init(_gpio, &gpio_init);

  return 0;
}

template <size_t BUF_SIZE>
off_t UartTx<BUF_SIZE>::llseek(OFile &ofile, off_t offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    return ofile.pos = offset;
  case SEEK_CUR:
    return ofile.pos += offset;
  default:
    return -EINVAL;
  }
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::start_dma_transfer(size_t count) const {
  LL_DMA_SetDataLength(_dma, _dma_stream, count);
  dma::clearFlagTC(_dma, _dma_stream);
  LL_USART_ClearFlag_TC(_usart);
  LL_DMA_EnableStream(_dma, _dma_stream);
}

template <size_t BUF_SIZE>
ssize_t UartTx<BUF_SIZE>::write(OFile &ofile, const char *buf, size_t count,
                      off_t &pos) {
  count = std::min(count, _buf.size());

  /* Wait for ongoing transfer to complete */
  while (!LL_USART_IsActiveFlag_TC(_usart)) {
    /* Except for non-blocking calls */
    if (ofile.flags & FNONBLOCK)
      return -EAGAIN;
  }

  /* Store the data to send in a contiguous local buffer */
  std::copy_n(buf, count, _buf.data());
  start_dma_transfer(count);

  pos += count;
  return count;
}

#endif // UARTTTX_TPP
