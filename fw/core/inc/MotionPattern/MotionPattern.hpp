/**
 * @file     MotionPattern.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.02.2025
 */

#ifndef MOTIONPATTERN_HPP
#define MOTIONPATTERN_HPP

#include "BStepper.h"
#include "flash.h"

template <size_t NMAX_MOTION_SEGMENTS>
class MotionPattern {
public:
  struct MotionSegment {
    BStepper::SpeedType milli_rev_per_minute;
    BStepper::StepCountType steps;
    BStepper::Direction direction;
  };

  MotionPattern(const flash::Sector &sec);

  const MotionSegment &operator[](size_t pos) const;
  const MotionSegment *data() const;

  const MotionSegment *begin() const;
  const MotionSegment *end() const;

  constexpr static size_t max_size() { return NMAX_MOTION_SEGMENTS; }
  size_t size() const;
  bool empty() const;

  void clear();
  const MotionSegment *emplaceBack(BStepper::SpeedType milli_rev_per_minute,
                                   BStepper::StepCountType steps,
                                   BStepper::Direction direction);
  const MotionSegment *emplaceBack(const MotionSegment &ms);

private:
  enum FlashChunkAttribute : uint8_t {
    ERASED = 0xFF,
    WRITTEN = 0xAA,
    DIRTY = 0x00
  };

  struct FlashChunkEntry {
    /* Motion Segment */
    BStepper::SpeedType milli_rev_per_minute;
    BStepper::StepCountType steps;
    BStepper::Direction direction;
    /* Packed with a chunk attribute */
    FlashChunkAttribute attr;
  };

  using FlashChunk = FlashChunkEntry[NMAX_MOTION_SEGMENTS];
  using CacheChunk = std::array<MotionSegment, NMAX_MOTION_SEGMENTS>;

  void erase();
  void markDirty();

  flash::Sector _sec;
  FlashChunk *_fchunk;
  size_t _fchunk_idx;
  CacheChunk _cchunk;
  size_t _n;
};

#include "MotionPattern.tpp"

#endif // MOTIONPATTERN_HPP
