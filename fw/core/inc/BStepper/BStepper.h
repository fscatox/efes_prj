/**
 * @file     BStepper.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     30.01.2025
 */

#ifndef BSTEPPER_H
#define BSTEPPER_H

#include "Translator.h"
#include "dma.h"
#include "gpio.h"
#include "tim.h"

class BStepper {
public:
  using StepCountType = Translator::StepCountType;
  using SpeedType = uint32_t;

  using Direction = Translator::Direction;
  using enum Translator::Direction;
  using StepType = Translator::StepType;
  using enum Translator::StepType;

  using PhasePins = Translator::PhasePins;
  struct Pinout {
    uint32_t en;
    Translator::Pinout ph;
  };

  BStepper(GPIO_TypeDef *gpio, TIM_TypeDef *tim, DMA_TypeDef *dma);
  void handler();

  void setPins(const Pinout &p);
  void setDMATransfer(uint32_t stream, uint32_t ch,
                      uint32_t stream_priority = LL_DMA_PRIORITY_VERYHIGH);

  void setResolution(uint16_t steps_per_rev);
  uint16_t getResolution() const;

  void init(uint32_t preempt = 0, uint32_t sub = 0);
  void updateClock();

  void enable() const;
  void disable() const;

  bool rotate(StepCountType steps, SpeedType milli_rev_per_minute,
              Direction d, bool block = false, StepType t = FULL);

private:
  /* DMA design forces to use advanced timers, which are 16 bit only */
  using TimRegType = uint16_t;
  using TimRCRType = uint8_t;

  bool calcTimeBase(SpeedType milli_rev_per_minute, StepType t, TimRegType &psc,
                    TimRegType &arr) const;

  GPIO_TypeDef *_gpio;
  TIM_TypeDef *_tim;
  DMA_TypeDef *_dma;
  uint32_t _dma_stream;
  uint32_t _dma_priority;
  uint32_t _dma_channel;

  Pinout _pins;
  Translator _tr;

  uint64_t _sixtyk_psc_clk_hz;
  uint16_t _steps_per_rev;

  volatile StepCountType _sw_reps;
  volatile TimRCRType _hw_reps;
};

#endif // BSTEPPER_H
