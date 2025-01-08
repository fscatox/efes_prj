/**
 * @file     PushButton.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     07.01.2025
 */

#include "PushButton.h"
#include "TimeBase.h"

#include "stm32f401xe.h"
#include "stm32f4xx_ll_exti.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_gpio.h"

uint32_t PushButton::_portNo() const {
  auto gpio_base = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(_port));
  return (gpio_base - AHB1PERIPH_BASE) >> 10;
}

uint32_t PushButton::_lineNo() const {
  return POSITION_VAL(_pin_msk);
}

PushButton::Edge_t PushButton::_trailing() const {
  return static_cast<Edge_t>(!_leading);
}

void PushButton::_enableTrig(Edge_t edge) const {
  if (edge == FALLING)
    LL_EXTI_EnableFallingTrig_0_31(_pin_msk);
  else
    LL_EXTI_EnableRisingTrig_0_31(_pin_msk);
}

void PushButton::_disableTrig(Edge_t edge) const {
  if (edge == FALLING)
    LL_EXTI_DisableFallingTrig_0_31(_pin_msk);
  else
    LL_EXTI_DisableRisingTrig_0_31(_pin_msk);
}

PushButton::PushButton(GPIO_TypeDef *port, uint32_t pin_msk,
                       Callback_t shortPressCbk, Callback_t longPressCbk,
                       Edge_t leading, TickType_t reject_ms, TickType_t press_ms)
    : _port(port), _pin_msk(pin_msk),
      _shortPressCbk(shortPressCbk), _longPressCbk(longPressCbk),
      _leading(leading), _reject_ms(reject_ms), _press_ms(press_ms),
      _state(RELEASED) {}

void PushButton::init() const {
  auto line_no = _lineNo();
  auto port_no = _portNo();

  uint32_t ll_syscfg_exti_line =
    ((0x000FU << ((line_no % 4) << 2)) << 16) | (line_no >> 2);

  /* Remap GPIO as external interrupt line */
  LL_SYSCFG_SetEXTISource(port_no, ll_syscfg_exti_line);
}

IRQn_Type PushButton::irqn() const {
  if (_lineNo() < 5)
    return static_cast<IRQn_Type>(EXTI0_IRQn + _lineNo());
  if (_lineNo() < 10)
    return EXTI9_5_IRQn;

  return EXTI15_10_IRQn;
}

void PushButton::_maskTrig() const {
  NVIC_DisableIRQ(irqn());
}

void PushButton::_unmaskTrig() const {
  NVIC_EnableIRQ(irqn());
}

void PushButton::enableIt() const {
  LL_EXTI_EnableIT_0_31(_pin_msk);
  _enableTrig(_leading);
}

void PushButton::extiHandler() {
  if (!LL_EXTI_ReadFlag_0_31(_pin_msk))
    return;

  _trig_tick = ms_ticks;

  if (_state == RELEASED) {
    _state = LTRIG;
    _disableTrig(_leading);
  } else if (_state == PRESSED) {
    _state = TTRIG;
    _disableTrig(_trailing());
  }

  LL_EXTI_ClearFlag_0_31(_pin_msk);
}

void PushButton::run() {

  if ((_state == LTRIG || _state == TTRIG) && (ms_ticks - _trig_tick > _reject_ms)) {
    bool level = LL_GPIO_IsInputPinSet(_port, _pin_msk);

    if (_state == LTRIG) {
      if (level != _leading) {
        _state = RELEASED;
        _enableTrig(_leading);
      } else {
        _state = PRESSED;
        _leading_tick = _trig_tick;
        _enableTrig(_trailing());
      }
    } else {
      if (level != _trailing()) {
        _state = PRESSED;
        _enableTrig(_trailing());
      } else {
        _shortPressCbk();
        _state = RELEASED;
        _enableTrig(_leading);
      }
    }
  }

  _maskTrig();
  if ((_state == PRESSED) && (ms_ticks - _leading_tick > _press_ms)) {
    _disableTrig(_trailing());
    _unmaskTrig();

    _longPressCbk();
    _state = RELEASED;
    _enableTrig(_leading);
  } else
    _unmaskTrig();
}

