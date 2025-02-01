/**
 * @file     UartTx.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.01.2025
 */

#ifndef UARTTX_HPP
#define UARTTX_HPP

#include "IFile.h"

#include "dma.h"
#include "gpio.h"
#include "usart.h"

#include <array>

template <size_t BUF_SIZE = 512>
class UartTx : public IFile {
public:
  UartTx(USART_TypeDef *usart, DMA_TypeDef *dma);

  void setPin(GPIO_TypeDef *gpio, uint32_t pin, uint32_t af);
  void setFrame(uint32_t parity, uint32_t stop);
  void setBaudRate(uint32_t baud_rate, uint32_t over_sampling);
  void setDMATransfer(uint32_t stream, uint32_t ch,
                      uint32_t stream_priority = LL_DMA_PRIORITY_LOW);

  int open(OFile &ofile) override;
  int close(OFile &ofile) override;

  off_t llseek(OFile &ofile, off_t offset, int whence) override;
  ssize_t write(OFile &ofile, const char *buf, size_t count, off_t &pos) override;

protected:
  using BufferType = std::array<uint8_t, BUF_SIZE>;

  bool isSending() const;
  bool wait(bool non_block = false); /*! not atomic */
  void startDMATransfer(size_t count) const; /*! from last memory address */
  void startDMATransfer(BufferType::const_iterator first, size_t count) const;

  USART_TypeDef *_usart;
  DMA_TypeDef *_dma;
  uint32_t _dma_stream;
  GPIO_TypeDef *_gpio;

  BufferType _buf;

private:
  uint32_t _gpio_pin;
  uint32_t _gpio_af;
  uint32_t _usart_parity;
  uint32_t _usart_stop;
  uint32_t _usart_baud_rate;
  uint32_t _usart_over_sampling;
  uint32_t _dma_priority;
  uint32_t _dma_channel;
};

#include "UartTx.tpp"

#endif // UARTTX_HPP
