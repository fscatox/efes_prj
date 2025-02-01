/**
 * @file     PushButton.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     07.01.2025
 */

#ifndef PUSHBUTTON_HPP
#define PUSHBUTTON_HPP

#include "CallbackUtils.hpp"
#include "stm32f4xx.h"

template <typename HwAlarm, bool FALLING_TRIGGER = true>
class PushButton {
public:
  using MilliSeconds = typename HwAlarm::MilliSeconds;

  PushButton(GPIO_TypeDef *gpio, uint32_t pin_mask, HwAlarm &hw_alarm,
             MilliSeconds reject = MilliSeconds{20},
             MilliSeconds long_press = MilliSeconds{800});
  void handler();

  void init(uint32_t preempt = 0, uint32_t sub = 0) const;
  void disable();
  bool enable();

  bool shortPress(bool disable = false);
  bool longPress(bool disable = false);

private:
  using CallbackType =
      MemFnCallback<PushButton, typename HwAlarm::ICallbackType::FnType>;

  enum State { OFF, IDLE, REJECTING, TRIGGERED, DETECTED_SHORT, DETECTED_LONG };
  enum Edge : bool { LEADING = true, TRAILING = false };

  void alarm();
  void enableTrig(Edge edge) const;
  void disableTrig(Edge edge) const;
  bool isEnabledTrig(Edge edge) const;

  GPIO_TypeDef *_gpio;
  uint32_t _pin_mask;
  HwAlarm &_hw_alarm;
  CallbackType _alarm_cb;

  volatile State _state;
  MilliSeconds _reject;
  MilliSeconds _long_press_residual;
};

#include "PushButton.tpp"

#endif // PUSHBUTTON_HPP
