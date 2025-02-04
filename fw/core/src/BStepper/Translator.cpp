/**
 * @file     Translator.cpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     30.01.2025
 */

#include "Translator.h"
#include <algorithm>
#include <debug.h>

bool Translator::Step::isHalf() const {
  return a == PhaseCurrent::OFF || b == PhaseCurrent::OFF;
}

void Translator::genHalfStepMaskSequence(HSMaskSequence &to, const Pinout &p) {
  constexpr auto hss = Half_Step_Sequence();

  /* Generate a BSRR mask for each half step knowing the phase currents */
  const auto it = std::transform(
      hss.begin(), hss.end(), to.begin(), [&p](const Step &s) -> uint32_t {
        return p.a.pos << (static_cast<uint16_t>(s.a) >> 8) |
               p.a.neg << (static_cast<uint16_t>(s.a) & 0xFFU) |
               p.b.pos << (static_cast<uint16_t>(s.b) >> 8) |
               p.b.neg << (static_cast<uint16_t>(s.b) & 0xFFU);
      });

  /* Add redundant masks to have n_half_steps contiguous steps
   * starting from any index [0, n_half_steps) */
  std::copy(to.begin(), it - 1, it);
}

void Translator::revHalfStepMaskSequence(HSMaskSequence &to,
                                         const HSMaskSequence &from) {
  /* Reverse-copy from redundant 0 */
  const auto it =
      std::copy_n(from.rend() - (n_half_steps + 1), n_half_steps, to.begin());

  /* Add redundant entries */
  std::copy(to.begin(), it - 1, it);
}

void Translator::genTables(const Pinout &p) {
  genHalfStepMaskSequence(_hs_mask[CCW], p);
  revHalfStepMaskSequence(_hs_mask[CW], _hs_mask[CCW]);

  /* skip entries coming from PhaseCurrent::OFF */
  for (size_t i = 0; i < FSMaskSequence{}.size(); ++i) {
    _fs_mask[CCW][i] = _hs_mask[CCW][i << 1];
    _fs_mask[CW][i] = _hs_mask[CW][i << 1];
  }
}

Translator::Translator() : _state(0), _hs_mask{}, _fs_mask{} {}

void Translator::setPins(const Pinout &p) { genTables(p); }

uint32_t Translator::setHome() {
  _state = 0;
  return _hs_mask[CCW][_state]; /* It's the same for HALF/FULL, CCW/CW*/
}

auto Translator::nextStep(Direction d, WStepCountType half_steps) const
    -> StepIndexType {
  return (d == CCW ? static_cast<WStepCountType>(_state) + half_steps
                   : static_cast<WStepCountType>(_state) - half_steps) &
         (n_half_steps - 1);
}

const uint32_t *Translator::advance(StepCountType steps, Direction d,
                                    StepType t) {
  /* When step type changes from HALF to FULL, the current step may be
   * a half step: compensate starting the movement from the next full step */
  if (t == FULL && Half_Step_Sequence()[_state].isHalf())
    _state = nextStep(d);

  /* Rotation starts from next step */
  StepIndexType hs_mask_idx = nextStep(d, t == HALF ? 1 : 2);

  /* Update with arrival state */
  _state =
      nextStep(d, t == HALF ? steps : static_cast<WStepCountType>(steps) << 1);

  return t == HALF ? _hs_mask[d].data() + hs_mask_idx
                   : _fs_mask[d].data() + (hs_mask_idx >> 1);
}
