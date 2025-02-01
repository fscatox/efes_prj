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

template <size_t BUF_SIZE>
UartTx<BUF_SIZE>::UartTx(USART_TypeDef *usart, DMA_TypeDef *dma)
    : _usart(usart), _dma(dma), _gpio(nullptr) {}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setPin(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af) {
  _gpio = gpio;
  _gpio_pin = pin;
  _gpio_af = af;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setFrame(uint32_t parity, uint32_t stop) {
  _usart_parity = parity;
  _usart_stop = stop;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setBaudRate(uint32_t baud_rate, uint32_t over_sampling) {
  _usart_baud_rate = baud_rate;
  _usart_over_sampling = over_sampling;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::setDMATransfer(uint32_t stream, uint32_t ch,
                            uint32_t stream_priority) {
  _dma_stream = stream;
  _dma_priority = stream_priority;
  _dma_channel = ch;
}

template <size_t BUF_SIZE>
int UartTx<BUF_SIZE>::open(OFile &ofile) {
  if (ofile.mode != FWRITE)
    return -EINVAL;

  /* Initialize GPIO peripheral */
  gpio::enableClock(_gpio);
  LL_GPIO_InitTypeDef gpio_init {
    .Pin = _gpio_pin,
    .Mode = LL_GPIO_MODE_ALTERNATE,
    .Speed = LL_GPIO_SPEED_FREQ_LOW,
    .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
    .Pull = LL_GPIO_PULL_NO,
    .Alternate = _gpio_af
  };
  LL_GPIO_Init(_gpio, &gpio_init);

  /* Initialize DMA peripheral */
  dma::enableClock(_dma);
  LL_DMA_InitTypeDef dma_init {
    .PeriphOrM2MSrcAddress = reinterpret_cast<uintptr_t>(&_usart->DR),
    .MemoryOrM2MDstAddress = reinterpret_cast<uintptr_t>(_buf.data()),
    .Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH,
    .Mode = LL_DMA_MODE_NORMAL,
    .PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT,
    .MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT,
    .PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE,
    .MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE,
    .Channel = _dma_channel,
    .Priority = _dma_priority,
    .FIFOMode = LL_DMA_FIFOMODE_DISABLE
  };
  LL_DMA_Init(_dma, _dma_stream, &dma_init);

  /* Initialize USART peripheral */
  usart::enableClock(_usart);
  LL_USART_InitTypeDef usart_init {
    .BaudRate = _usart_baud_rate,
    .DataWidth = _usart_parity == LL_USART_PARITY_NONE ? LL_USART_DATAWIDTH_8B
                                                       : LL_USART_DATAWIDTH_9B,
    .StopBits = _usart_stop,
    .Parity = _usart_parity,
    .TransferDirection = LL_USART_DIRECTION_TX,
    .HardwareFlowControl = LL_USART_HWCONTROL_NONE,
    .OverSampling = _usart_over_sampling
  };
  LL_USART_Init(_usart, &usart_init);

  /* Enable USART in DMA mode */
  LL_USART_EnableDMAReq_TX(_usart);
  LL_USART_Enable(_usart);

  return 0;
}

template <size_t BUF_SIZE>
bool UartTx<BUF_SIZE>::isSending() const {
  return !LL_USART_IsActiveFlag_TC(_usart);
}

template <size_t BUF_SIZE>
bool UartTx<BUF_SIZE>::wait(bool non_block) {
  while (isSending()) {
    if (non_block)
      return false;
  }
  LL_USART_ClearFlag_TC(_usart);
  return true;
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::startDMATransfer(size_t count) const {
  LL_DMA_SetDataLength(_dma, _dma_stream, count);
  dma::clearFlagTC(_dma, _dma_stream);
  LL_DMA_EnableStream(_dma, _dma_stream);
}

template <size_t BUF_SIZE>
void UartTx<BUF_SIZE>::startDMATransfer(
    typename BufferType::const_iterator first, size_t count) const {
  LL_DMA_SetMemoryAddress(_dma, _dma_stream,
    reinterpret_cast<uintptr_t>(&*first));
  startDMATransfer(count);
}

template <size_t BUF_SIZE>
int UartTx<BUF_SIZE>::close([[maybe_unused]] OFile &ofile) {
  LL_GPIO_InitTypeDef gpio_init {
    .Pin = _gpio_pin,
    .Mode = LL_GPIO_MODE_ANALOG,
  };

  /* Wait for ongoing transfer to complete */
  while (isSending());

  /* De-init peripherals */
  LL_USART_DeInit(_usart);
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
ssize_t UartTx<BUF_SIZE>::write(OFile &ofile, const char *buf, size_t count,
                      off_t &pos) {
  count = std::min(count, _buf.size());

  /* Wait for ongoing transfer to complete */
  if (!wait(ofile.flags & FNONBLOCK))
    return -EAGAIN;

  /* Move data to local contiguous buffer */
  std::copy_n(buf, count, _buf.data());
  startDMATransfer(count);

  pos += count;
  return count;
}

#endif // UARTTTX_TPP
