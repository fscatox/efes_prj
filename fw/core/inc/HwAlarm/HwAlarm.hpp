/**
 * @file     HwAlarm.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     24.01.2025
 */

#ifndef HWALARM_HPP
#define HWALARM_HPP

#include <array>
#include <chrono>

#include "CallbackUtils.hpp"
#include "tim.h"

template <uintptr_t TimBase>
class HwAlarm {
 public:
  using DurationRep = uint64_t;
  using NanoSeconds = std::chrono::duration<DurationRep, std::nano>;
  using MicroSeconds = std::chrono::duration<DurationRep, std::micro>;
  using MilliSeconds = std::chrono::duration<DurationRep, std::milli>;
  using ICallbackType = ICallback<void()>;
  using Cnt = std::conditional_t<tim::is32Bit(TimBase), uint32_t, uint16_t>;

  enum AlarmState {
    INVALID_CALLBACK = -4,
    CHANNELS_BUSY,
    INVALID_DELAY,
    DELAY_TOO_SHORT,
    STARTED = 0,
    STOPPED,
    CHANGED
  };

  HwAlarm();
  void handler();

  bool init(const NanoSeconds &t_cnt, uint32_t preempt = 0, uint32_t sub = 0);
  bool setResolution(const NanoSeconds &tick);

  /**
   * @brief Start an alarm from current CNT
   * @param delay Time interval between consecutive alarm firings
   * @param icb Callback object @ref FnCallback, @ref MemFnCallback
   * @param reps Periodic (enabled with 0 or numeric max) or Finite-repetitions
   * alarm (1, 2, ..., max - 1)
   * @return Either @ref AlarmState SET, or an error enum value (< 0)
   */
  AlarmState setAlarm(const NanoSeconds &delay, const ICallbackType *icb,
                      uint32_t reps = 1);

  /**
   * @brief Change a running alarm
   * @param icb Callback object of the alarm to modify
   * @param reps Modify repetitions: 0 for stop; 1, 2, ..., max-1 for finite
   * number; numeric max for periodic
   * @param delay Modify time interval between consecutive firings, or don't
   * (default 0)
   * @param icb_new Modify callback object, or don't (default icb)
   * @return Either @ref AlarmState SET, or an error enum value (< 0)
   */
  AlarmState setAlarm(const ICallbackType *icb, uint32_t reps,
                      const NanoSeconds &delay = NanoSeconds::zero(),
                      const ICallbackType *icb_new = nullptr);

  void delay(const NanoSeconds &t) const;

 private:
  static inline auto _tim = reinterpret_cast<TIM_TypeDef *>(TimBase);
  static constexpr auto _nch = tim::getNChannels(TimBase);

  using Psc = uint16_t;

  struct Alarm {
    const ICallbackType *volatile icb;
    volatile uint32_t reps;
    volatile Cnt ticks;
  };
  using AlarmContainer = std::array<Alarm, _nch>;

  bool calcTimeBase(const NanoSeconds &t_cnt, Psc &psc);
  size_t getChannel() const;
  void freeChannel(size_t ch);

  AlarmContainer _alarms;
  /*volatile*/ uint32_t _psc_clk;
  /*volatile*/ DurationRep _psc_plus_one_times_den;
  /*volatile*/ DurationRep _psc_plus_one_times_half_den;
};

#include "HwAlarm.tpp"

#endif  // HWALARM_HPP
