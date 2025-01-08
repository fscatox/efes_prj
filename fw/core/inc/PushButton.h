/**
 * @file     PushButton.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     07.01.2025
 */

#ifndef PUSHBUTTON_H
#define PUSHBUTTON_H

#include "TimeBase.h"
#include "stm32f4xx.h"

class PushButton {
  public:
    typedef enum {FALLING, RISING} Edge_t;
    typedef void (*Callback_t)();

    PushButton(GPIO_TypeDef *port, uint32_t pin_msk,
               Callback_t shortPressCbk, Callback_t longPressCbk,
               Edge_t leading = FALLING,
               TickType_t reject_ms = 20, TickType_t press_ms = 800);

    void init() const;

    void enableIt() const;
    IRQn_Type irqn() const;
    void extiHandler();

    void run();

  private:
    typedef enum {RELEASED, LTRIG, PRESSED, TTRIG} _state_t;

    uint32_t _portNo() const;
    uint32_t _lineNo() const;
    Edge_t _trailing() const;

    void _enableTrig(Edge_t edge) const;
    void _disableTrig(Edge_t edge) const;
    void _maskTrig() const;
    void _unmaskTrig() const;

    GPIO_TypeDef *_port;
    uint32_t _pin_msk;
    Callback_t _shortPressCbk;
    Callback_t _longPressCbk;
    Edge_t _leading;
    TickType_t _reject_ms;
    TickType_t _press_ms;

    volatile _state_t _state;
    volatile TickType_t _trig_tick;
    TickType_t _leading_tick;
};

#endif  // PUSHBUTTON_H
