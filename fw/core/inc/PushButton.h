/**
 * @file     PushButton.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     07.01.2025
 */

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "core.h"
#include "stm32f4xx.h"

class PushButton {
public:
  enum Edge { FALLING = 0, RISING };
  using Callback = auto (*)() -> void;

  PushButton(GPIO_TypeDef *port, uint32_t pin_msk, Callback shortPressCbk,
             Callback longPressCbk, Edge leading = FALLING,
             TickType_t reject_ms = 20, TickType_t press_ms = 800);

  void init() const;

  void enableIt(uint32_t preempt = 0, uint32_t sub = 0) const;
  void extiHandler();

  void run();

private:
  typedef enum { RELEASED, LTRIG, PRESSED, TTRIG } _state_t;

  uint32_t _lineNo() const;
  Edge _trailing() const;

  IRQn_Type _irqn() const;
  void _enableTrig(Edge edge) const;
  void _disableTrig(Edge edge) const;
  void _maskTrig() const;
  void _unmaskTrig() const;

  GPIO_TypeDef *_port;
  uint32_t _pin_msk;
  Callback _shortPressCbk;
  Callback _longPressCbk;
  Edge _leading;
  TickType_t _reject_ms;
  TickType_t _press_ms;

  volatile _state_t _state;
  volatile TickType_t _trig_tick;
  TickType_t _leading_tick;
};

#endif // PUSHBUTTON_H
