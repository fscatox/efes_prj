/**
 * @file     UartTx.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#include "UartTx.h"

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "stm32f4xx_utils.h"

UartTx::UartTx(USART_TypeDef *usart, DMA_TypeDef *dma)
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
                 .Pull = LL_GPIO_PULL_NO} {
  _buf.reserve(BUF_SIZE);
}

void UartTx::setPin(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af) {
  _gpio = gpio;
  _gpio_init.Pin = pin;
  _gpio_init.Alternate = af;
}

void UartTx::setFrame(uint32_t data_width, uint32_t parity, uint32_t stop) {
  _usart_init.DataWidth = data_width;
  _usart_init.Parity = parity;
  _usart_init.StopBits = stop;
}

void UartTx::setBaudRate(uint32_t baud_rate, uint32_t over_sampling) {
  _usart_init.BaudRate = baud_rate;
  _usart_init.OverSampling = over_sampling;
}

void UartTx::setDMATransfer(uint32_t stream, uint32_t ch,
                            uint32_t stream_priority) {
  _dma_stream = stream;
  _dma_init.Priority = stream_priority;
  _dma_init.Channel = ch;
  _dma_init.MemoryOrM2MDstAddress = reinterpret_cast<uintptr_t>(_buf.data());
}

int UartTx::open(const OFile &ofile) {
  if (ofile.mode != FWRITE)
    return -EINVAL;

  /* Initialize GPIO peripheral */
  GPIO_enableClock(_gpio);
  LL_GPIO_Init(_gpio, &_gpio_init);

  /* Initialize DMA peripheral */
  DMA_enableClock(_dma);
  LL_DMA_Init(_dma, _dma_stream, &_dma_init);

  /* Initialize USART peripheral */
  USART_enableClock(_usart);
  LL_USART_Init(_usart, &_usart_init);

  /* Enable USART in DMA mode */
  LL_USART_EnableDMAReq_TX(_usart);
  LL_USART_Enable(_usart);

  return 0;
}

int UartTx::close([[maybe_unused]] const OFile &ofile) {
  return -ENOTSUP;
}

off_t UartTx::llseek(OFile &ofile, off_t offset, int whence) {
  switch (whence) {
  case SEEK_SET:
    return ofile.pos = offset;
  case SEEK_CUR:
    return ofile.pos += offset;
  default:
    return -EINVAL;
  }
}

ssize_t UartTx::write(const OFile &ofile, const void *buf, size_t count,
                      off_t &pos) {
  auto cbuf = static_cast<Buffer::const_pointer>(buf);
  count = std::min(count, BUF_SIZE);

  /* Wait for ongoing transfer to complete */
  while (!LL_USART_IsActiveFlag_TC(_usart)) {
    /* Except for non-blocking calls */
    if (ofile.flags & FNONBLOCK)
      return -EAGAIN;
  }

  /* Store the data to send in a contiguous local buffer */
  _buf.assign(cbuf, cbuf + count);

  /* Configure DMA transfer */
  LL_DMA_SetDataLength(_dma, _dma_stream, count);
  LL_DMA_ClearFlag_TC[_dma_stream](_dma);
  LL_DMA_EnableStream(_dma, _dma_stream);

  pos += count;
  return count;
}
