/**
 * @file     MotionPattern.tpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.02.2025
 */

#ifndef MOTIONPATTERN_TPP
#define MOTIONPATTERN_TPP

template <size_t NMAX_MOTION_SEGMENTS>
void MotionPattern<NMAX_MOTION_SEGMENTS>::erase() {
  if (!flash::unlock()) {
    PRINTE("flash::unlock() failed. Forcing reset...");
    exit(-3);
  }

  setOperation(flash::Op::SER);
  setParallelism(flash::PSize::x32);
  setSector(_sec);
  flash::startErase();

  /* stall and lock */
  flash::lock();
  PRINTD("Sector S%d erased", static_cast<uint32_t>(_sec));
}

template <size_t NMAX_MOTION_SEGMENTS>
MotionPattern<NMAX_MOTION_SEGMENTS>::MotionPattern(const flash::Sector &sec)
    : _sec(sec), _fchunk(reinterpret_cast<FlashChunk *>(getBaseAddr(sec))),
      _fchunk_idx(0), _n(0) {
  const auto sec_size = getSize(_sec);
  const auto n_fchunks = sec_size / sizeof(FlashChunk);

  PRINTD("Sector S%d (0x%08x, %uB): n_fchunks = %u, sizeof(FlashChunk) = %uB",
         static_cast<uint32_t>(_sec), _fchunk, sec_size, n_fchunks,
         sizeof(FlashChunk));

  /* Find first non-dirty chunk */
  while (_fchunk_idx < n_fchunks && (*_fchunk)->attr == DIRTY) {
    ++_fchunk_idx;
    ++_fchunk;
  }

  /* All chunks dirty? Erase the sector */
  if (_fchunk_idx == n_fchunks) {
    erase();
    _fchunk_idx = 0;
    _fchunk = reinterpret_cast<FlashChunk *>(getBaseAddr(_sec));
  }

  /* Load cache from written entries */
  while (_n < max_size() && (*_fchunk)[_n].attr == WRITTEN) {
    _cchunk[_n].milli_rev_per_minute = (*_fchunk)[_n].milli_rev_per_minute;
    _cchunk[_n].steps = (*_fchunk)[_n].steps;
    _cchunk[_n].direction = (*_fchunk)[_n].direction;
    ++_n;
  }
}

template <size_t NMAX_MOTION_SEGMENTS>
auto MotionPattern<NMAX_MOTION_SEGMENTS>::operator[](size_t pos) const
    -> const MotionSegment & {
  return _cchunk[pos];
}

template <size_t NMAX_MOTION_SEGMENTS>
auto MotionPattern<NMAX_MOTION_SEGMENTS>::data() const
    -> const MotionSegment * {
  return _cchunk.data();
}

template <size_t NMAX_MOTION_SEGMENTS>
auto MotionPattern<NMAX_MOTION_SEGMENTS>::begin() const
    -> const MotionSegment * {
  return _cchunk.begin();
}

template <size_t NMAX_MOTION_SEGMENTS>
auto MotionPattern<NMAX_MOTION_SEGMENTS>::end() const -> const MotionSegment * {
  return _cchunk.begin() + _n;
}

template <size_t NMAX_MOTION_SEGMENTS>
size_t MotionPattern<NMAX_MOTION_SEGMENTS>::size() const {
  return _n;
}

template <size_t NMAX_MOTION_SEGMENTS>
void MotionPattern<NMAX_MOTION_SEGMENTS>::markDirty() {
  if (!flash::unlock()) {
    PRINTE("flash::unlock() failed. Forcing reset...");
    exit(-3);
  }

  setOperation(flash::Op::PG);
  flash::setParallelism(FlashChunkAttribute{});
  (*_fchunk)->attr = DIRTY;

  /* stall and lock */
  flash::lock();
  if (isActive(flash::PGERR)) {
    PRINTE("Failed marking fchunk %u (0x%08x) DIRTY. Forcing reset...",
           _fchunk_idx, _fchunk);
    exit(-3);
  }
  PRINTD("fchunk %u (0x%08x) marked DIRTY", _fchunk_idx, _fchunk);
}

template <size_t NMAX_MOTION_SEGMENTS>
void MotionPattern<NMAX_MOTION_SEGMENTS>::clear() {
  const auto max_fchunk_idx = getSize(_sec) / sizeof(FlashChunk) - 1;
  PRINTD("Clearing fchunk %u/%u ...", _fchunk_idx, max_fchunk_idx);

  if (_fchunk_idx == max_fchunk_idx) {
    /* marking the chunk dirty leaves no usable fchunk */
    erase();
    _fchunk_idx = 0;
    _fchunk = reinterpret_cast<FlashChunk *>(getBaseAddr(_sec));
  } else {
    /* next chunk is erased */
    markDirty();
    ++_fchunk_idx;
    ++_fchunk;
  }

  /* Clear cache */
  _n = 0;
}

template <size_t NMAX_MOTION_SEGMENTS>
auto MotionPattern<NMAX_MOTION_SEGMENTS>::emplaceBack(
    BStepper::SpeedType milli_rev_per_minute, BStepper::StepCountType steps,
    BStepper::Direction direction) -> const MotionSegment * {

  if (!flash::unlock()) {
    PRINTE("flash::unlock() failed. Unable to set Op::PG");
    return nullptr;
  }

  setOperation(flash::Op::PG);

  flash::setParallelism(BStepper::SpeedType{});
  (*_fchunk)[_n].milli_rev_per_minute = milli_rev_per_minute;

  flash::setParallelism(BStepper::StepCountType{});
  (*_fchunk)[_n].steps = steps;

  flash::setParallelism(BStepper::Direction{});
  (*_fchunk)[_n].direction = direction;

  if (isActive(flash::PGERR, true)) {
    PRINTE("Programming (*0x%08x)[%u] failed", _fchunk, _n);
    return nullptr;
  }

  /* Fail-safe update by changing the entry attribute last */
  flash::setParallelism(FlashChunkAttribute{});
  (*_fchunk)[_n].attr = WRITTEN;
  flash::lock();

  if (isActive(flash::PGERR, true)) {
    PRINTE("Programming (*0x%08x)[%u] failed", _fchunk, _n);
    return nullptr;
  }

  /* Cache update */
  _cchunk[_n].milli_rev_per_minute = milli_rev_per_minute;
  _cchunk[_n].steps = steps;
  _cchunk[_n].direction = direction;
  return &_cchunk[_n++];
}

#endif // MOTIONPATTERN_TPP