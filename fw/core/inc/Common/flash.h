/**
 * @file     flash.h
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     06.02.2025
 */

#ifndef FLASH_H
#define FLASH_H

#include "stm32f4xx.h"
#include <cstddef>

namespace flash {

enum Flag : uint32_t {
  PGAERR = FLASH_SR_PGAERR,
  PGPERR = FLASH_SR_PGPERR,
  PGSERR = FLASH_SR_PGSERR,
  PGERR = FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR,
  OPERR = FLASH_SR_SOP,
  EOP = FLASH_SR_EOP,
};
enum It : uint32_t {
  ERRI = FLASH_CR_ERRIE,
  EOPI = FLASH_CR_EOPIE,
};

enum class PSize : uint32_t { x8 = 0, x16, x32, x64 };
enum class Sector : uint32_t { S0 = 0, S1, S2, S3, S4, S5, S6, S7 };
enum class Op : uint32_t {
  MER = FLASH_CR_MER,
  SER = FLASH_CR_SER,
  PG = FLASH_CR_PG,
};

constexpr uintptr_t getBaseAddr(const Sector &s) {
  switch (s) {
  case Sector::S0:
    return 0x0800'0000U;
  case Sector::S1:
    return 0x0800'4000U;
  case Sector::S2:
    return 0x0800'8000U;
  case Sector::S3:
    return 0x0800'C000U;
  case Sector::S4:
    return 0x0801'0000U;
  case Sector::S5:
    return 0x0802'0000U;
  case Sector::S6:
    return 0x0804'0000U;
  case Sector::S7:
    return 0x0806'0000U;
  }
  return 0;
}

constexpr size_t getSize(const Sector &s) {
  switch (s) {
  case Sector::S0:
  case Sector::S1:
  case Sector::S2:
  case Sector::S3:
    return 0x1UL << 14;
  case Sector::S4:
    return 0x1UL << 16;
  case Sector::S5:
  case Sector::S6:
  case Sector::S7:
    return 0x1UL << 17;
  }
  return 0;
}

bool isBusy();
bool isActive(const Flag &f, bool clear = false);
bool isEnabledIt(const It &it);
bool isLocked();

bool unlock();
void lock();

void enableIt(const It &it);
void disableIt(const It &it);

template <typename T>
bool setParallelism(const T &t) {
  PSize p;
  if (sizeof(T) == 1)
    p = PSize::x8;
  else if (sizeof(T) == 2)
    p = PSize::x16;
  else if (sizeof(T) == 4)
    p = PSize::x32;
  else
    return false;

  setParallelism(p);
  return true;
}
template <>
inline bool setParallelism(const PSize &p) {
  FLASH->CR &= ~FLASH_CR_PSIZE;
  FLASH->CR |= static_cast<uint32_t>(p) << FLASH_CR_PSIZE_Pos;
  return true;
}

void setSector(const Sector &s);

void setOperation(const Op &op);
void startErase();
void clearOperation();

} // namespace flash

#endif // FLASH_H
