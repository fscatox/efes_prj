/**
 * @file     Translator.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     30.01.2025
 */

#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "PhaseCurrent.h"
#include <array>

class Translator {
public:
  using StepCountType = uint16_t;

  struct PhasePins {
    uint32_t pos;
    uint32_t neg;
  };
  struct Pinout {
    PhasePins a;
    PhasePins b;
  };

  enum Direction { CCW = 0, CW };
  enum StepType { FULL = 0, HALF };

  Translator();
  void setPins(const Pinout &p);

  uint32_t setHome();
  static constexpr uint8_t getSequenceLen(StepType t) {
    return n_half_steps >> (HALF - t);
  }
  const uint32_t *advance(StepCountType steps, Direction d = CCW,
                          StepType t = FULL);

private:
  using WStepCountType = uint32_t;
  using StepIndexType = uint8_t;

  struct Step {
    PhaseCurrent a;
    PhaseCurrent b;

    bool isHalf() const;
    bool operator==(const Step &) const = default;
  };

  static constexpr StepIndexType n_half_steps = 8;
  using HSMaskSequence = std::array<uint32_t, 2 * n_half_steps - 1>;
  using FSMaskSequence = std::array<uint32_t, n_half_steps - 1>;

  static constexpr std::array<Step, n_half_steps> Half_Step_Sequence() {
    return {{{PhaseCurrent::IN, PhaseCurrent::IN},
             {PhaseCurrent::IN, PhaseCurrent::OFF},
             {PhaseCurrent::IN, PhaseCurrent::OUT},
             {PhaseCurrent::OFF, PhaseCurrent::OUT},
             {PhaseCurrent::OUT, PhaseCurrent::OUT},
             {PhaseCurrent::OUT, PhaseCurrent::OFF},
             {PhaseCurrent::OUT, PhaseCurrent::IN},
             {PhaseCurrent::OFF, PhaseCurrent::IN}}};
  }
  static void genHalfStepMaskSequence(HSMaskSequence &to, const Pinout &p);
  static void revHalfStepMaskSequence(HSMaskSequence &to,
                                      const HSMaskSequence &from);

  void genTables(const Pinout &p);
  StepIndexType nextStep(Direction d, WStepCountType half_steps = 1) const;

  StepIndexType _state;
  std::array<HSMaskSequence, 1 + CW> _hs_mask;
  std::array<FSMaskSequence, 1 + CW> _fs_mask;
};

#endif // TRANSLATOR_H
