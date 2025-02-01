/**
 * @file     PushButton.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     07.01.2025
 */
#ifndef PUSHBUTTON_TPP
#define PUSHBUTTON_TPP

#include "gpio.h"
#include "exti.h"

template <typename HwAlarm, bool FALLING_TRIGGER>
PushButton<HwAlarm, FALLING_TRIGGER>::PushButton(GPIO_TypeDef *gpio,
                                                uint32_t pin_mask,
                                                HwAlarm &hw_alarm,
                                                MilliSeconds reject,
                                                MilliSeconds long_press)
    : _gpio(gpio), _pin_mask(pin_mask), _hw_alarm(hw_alarm),
      _alarm_cb(this, &PushButton::alarm), _state(OFF), _reject(reject),
      _long_press_residual(long_press - reject) {}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::enableTrig(Edge edge) const {
  if (edge == FALLING_TRIGGER)
    LL_EXTI_EnableFallingTrig_0_31(_pin_mask);
  else
    LL_EXTI_EnableRisingTrig_0_31(_pin_mask);
}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::disableTrig(Edge edge) const {
  if (edge == FALLING_TRIGGER)
    LL_EXTI_DisableFallingTrig_0_31(_pin_mask);
  else
    LL_EXTI_DisableRisingTrig_0_31(_pin_mask);

  /* safe to clear: only one detector enabled at a time */
  LL_EXTI_ClearFlag_0_31(_pin_mask);
}

template <typename HwAlarm, bool FALLING_TRIGGER>
bool PushButton<HwAlarm, FALLING_TRIGGER>::isEnabledTrig(Edge edge) const {
  return edge == FALLING_TRIGGER ? LL_EXTI_IsEnabledFallingTrig_0_31(_pin_mask)
                                : LL_EXTI_IsEnabledRisingTrig_0_31(_pin_mask);
}

template <typename HwAlarm, bool FALLING_TRIGGER>
bool PushButton<HwAlarm, FALLING_TRIGGER>::enable() {
  if (_state != OFF)
    return false;

  _state = IDLE;
  enableTrig(LEADING);
  LL_EXTI_EnableIT_0_31(_pin_mask);
  return true;
}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::disable() {
  LL_EXTI_DisableIT_0_31(_pin_mask);
  _hw_alarm.setAlarm(&_alarm_cb, 0);
  disableTrig(LEADING);
  disableTrig(TRAILING);
  _state = OFF;
}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::init(uint32_t preempt,
                                               uint32_t sub) const {
  gpio::enableClock(_gpio);

  /* route GPIO to edge detector */
  exti::map(_gpio, _pin_mask);

  /* register at NVIC side */
  NVIC_SetPriority(
      exti::getIRQn(_pin_mask),
      NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt, sub));
  NVIC_EnableIRQ(exti::getIRQn(_pin_mask));
}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::handler() {
  if (!LL_EXTI_ReadFlag_0_31(_pin_mask))
    return;

  if (isEnabledTrig(LEADING)) {
    /* Initialize and start new alarm, while not sensitive to edges
     * (the 2nd repetition is there just to be lengthened/stopped */
    _hw_alarm.setAlarm(_reject, &_alarm_cb, 2);
    _state = REJECTING;
    disableTrig(LEADING);
  } else {
    /* Trailing edge has arrived before the alarm could fire */
    _hw_alarm.setAlarm(&_alarm_cb, 0);
    _state = DETECTED_SHORT;
    disableTrig(TRAILING);
  }
}

template <typename HwAlarm, bool FALLING_TRIGGER>
void PushButton<HwAlarm, FALLING_TRIGGER>::alarm() {
  if (_state == REJECTING) {
    if (LL_GPIO_IsInputPinSet(_gpio, _pin_mask) != FALLING_TRIGGER) {
      /* actual edge (level did change) */
      _hw_alarm.setAlarm(&_alarm_cb, 1, _long_press_residual);
      _state = TRIGGERED;
      enableTrig(TRAILING);
    } else {
      /* spurious edge */
      _hw_alarm.setAlarm(&_alarm_cb, 0);
      _state = IDLE;
      enableTrig(LEADING);
    }
  } else if (_state == TRIGGERED) {
    /* Alarm fired before the trailing edge */
    _state = DETECTED_LONG;
    disableTrig(TRAILING);
  }
}

template <typename HwAlarm, bool FALLING_TRIGGER>
bool PushButton<HwAlarm, FALLING_TRIGGER>::shortPress(bool disable) {
  if (_state != DETECTED_SHORT)
    return false;

  _state = IDLE;
  if (!disable)
    enableTrig(LEADING);

  return true;
}

template <typename HwAlarm, bool FALLING_TRIGGER>
bool PushButton<HwAlarm, FALLING_TRIGGER>::longPress(bool disable) {
  if (_state != DETECTED_LONG)
    return false;

  _state = IDLE;
  if (!disable)
    enableTrig(LEADING);

  return true;
}

#endif //PUSHBUTTON_TPP
